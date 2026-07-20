from maix import image, display, time, app, uart

disp = display.Display()
img = image.Image(disp.width(), disp.height())
msg = "Loading ..."
size = image.string_size(msg, scale=1.5, thickness=2)
img.draw_string(
    (img.width() - size.width()) // 2,
    (img.height() - size.height()) // 2,
    msg,
    scale=1.5,
    thickness=2
)
disp.show(img)

from find_circle import FindRectCircle

SHOW_DEBUG = True

# =========================
# 串口参数
# 按你的实际串口修改
# =========================
UART_DEV = "/dev/ttyS0"
UART_BAUD = 115200

# 串口对象
stm32_uart = uart.UART(UART_DEV, UART_BAUD)


def send_target_error(err_pitch, err_yaw):
    """
    发送目标误差给 STM32
    这里将浮点误差放大 100 倍转为整数发送，方便 STM32 解析
    协议格式:
        $pitch_err,yaw_err#
    例如:
        $123,-256#
    表示:
        pitch_err = 1.23
        yaw_err   = -2.56
    """
    p = int(err_pitch * 100)
    y = int(err_yaw * 100)
    msg = "${},{}#".format(p, y)
    stm32_uart.write(msg.encode())


def send_target_lost():
    """
    发送丢失目标标志
    """
    stm32_uart.write(b"$9999,9999#")


class Target:
    def __init__(self, out_range_pitch: float, out_range_yaw: float,
                 ignore_limit: float, disp: display.Display):
        print("Target init: before FindRectCircle")

        self.pitch = 0.0
        self.yaw = 0.0
        self.out_range_pitch = out_range_pitch
        self.out_range_yaw = out_range_yaw
        self.ignore = ignore_limit

        self.finder = FindRectCircle(disp)

        print("Target init: after FindRectCircle")

        self.w, self.h = self.finder.get_res()
        self.updated = False

    def _get_target_err_pixels(self):
        """
        从视觉模块获取像素误差
        返回 err_center = (err_x, err_y)
        """
        if SHOW_DEBUG:
            print("Target: before finder.run()")

        circle_center, screen_center, err_center, circle3, update = self.finder.run()

        if SHOW_DEBUG:
            print("Target: after finder.run()")

        self.updated = update
        return err_center

    def get_target_err(self):
        """
        将像素误差映射为归一化后的 pitch/yaw 误差
        """
        err_x, err_y = self._get_target_err_pixels()

        # 注意：
        # err_y 对应 pitch（上下）
        # err_x 对应 yaw（左右）
        self.pitch = err_y / self.h * self.out_range_pitch
        self.yaw = err_x / self.w * self.out_range_yaw

        # 小误差忽略，避免抖动
        if abs(self.pitch) < self.out_range_pitch * self.ignore:
            self.pitch = 0.0
        if abs(self.yaw) < self.out_range_yaw * self.ignore:
            self.yaw = 0.0

        return self.pitch, self.yaw, self.updated


def main(disp):
    print("step 1: enter main")

    # =========================
    # 目标误差映射参数
    # 这里不是 PID 参数
    # 而是把图像误差缩放成发给 STM32 的控制误差范围
    # =========================
    target_err_range_pitch = 10.0
    target_err_range_yaw = 10.0
    target_ignore_limit = 0.02

    print("step 2: before Target")
    target = Target(
        target_err_range_pitch,
        target_err_range_yaw,
        target_ignore_limit,
        disp
    )
    print("step 3: after Target")

    # 控制发送周期
    ltime = time.ticks_ms()
    lost_count = 0

    while not app.need_exit():
        # 控制循环周期，约 100Hz
        if time.ticks_ms() - ltime < 10:
            continue
        ltime = time.ticks_ms()

        if SHOW_DEBUG:
            print("step 4: loop begin")

        err_pitch, err_yaw, updated = target.get_target_err()

        if SHOW_DEBUG:
            print("step 5: after get_target_err")

        if updated:
            lost_count = 0

            # 发给 STM32
            send_target_error(err_pitch, err_yaw)

            if SHOW_DEBUG:
                print(
                    "send -> pitch_err:{:.2f}, yaw_err:{:.2f}".format(
                        err_pitch, err_yaw
                    )
                )
        else:
            lost_count += 1
            send_target_lost()

            if SHOW_DEBUG:
                print("target lost, lost_count =", lost_count)


if __name__ == '__main__':
    try:
        main(disp)
    except Exception:
        import traceback
        msg = traceback.format_exc()
        print(msg)

        img = image.Image(disp.width(), disp.height())
        img.draw_string(0, 0, msg, image.COLOR_WHITE)
        disp.show(img)

        while not app.need_exit():
            time.sleep_ms(100)