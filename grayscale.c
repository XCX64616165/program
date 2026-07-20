#include "grayscale.h"
#include "zf_common_headfile.h"

int16 xunji_error = 0;
int16 xunji_pid_out = 0;
uint16 gray_value = 0;

static float last_error = 0;
static float integral_error = 0;

/*
 * 记录小车上一次实际转向方向
 * -1：上一次左转
 *  0：上一次直行
 *  1：上一次右转
 */
static int8 last_turn_dir = 0;

/*
 * 丢线标志
 */
static uint8 xunji_lost_flag = 0;

/*
限幅函数
*/
static int16 limit_int16(int16 value, int16 min, int16 max)
{
    if(value > max) return max;
    if(value < min) return min;
    return value;
}

/*
 * 将原始电平转换成是否检测到黑线
 */
static uint8 gray_is_active(uint8 level)
{
#if GRAY_ACTIVE_LEVEL
    return level ? 1 : 0;
#else
    return level ? 0 : 1;
#endif
}

/*
 * 灰度传感器初始化
 */
void Xunji_Init(void)
{
    gpio_init(H1,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H2,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H3,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H4,  GPI, GPIO_HIGH, GPI_PULL_UP);
    //gpio_init(H5,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H6,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H7,  GPI, GPIO_HIGH, GPI_PULL_UP);
    //gpio_init(H8,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H9,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H10, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H11, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(H12, GPI, GPIO_HIGH, GPI_PULL_UP);
}

/*
 * 读取 12 路灰度状态
 * 返回值 bit11~bit0 对应 H1~H12
 *
 * 例如：
 * H1 检测到黑线，则 bit11 = 1
 * H12 检测到黑线，则 bit0 = 1
 */
uint16 Gray_Read(void)
{
    uint8 h[10];

    h[0]  = gray_is_active(gpio_get_level(H1));
    h[1]  = gray_is_active(gpio_get_level(H2));
    h[2]  = gray_is_active(gpio_get_level(H3));
    h[3]  = gray_is_active(gpio_get_level(H4));
    //h[4]  = gray_is_active(gpio_get_level(H5));
	//h[4]  =  0;//因为12路灰度传感器OUT5是坏的
    h[4]  = gray_is_active(gpio_get_level(H6));
    h[5]  = gray_is_active(gpio_get_level(H7));
    //h[7]  = gray_is_active(gpio_get_level(H8));
	//h[7]  = 0;//为了抵消H5损坏带来的权重计算影响，对称去掉H8
    h[6]  = gray_is_active(gpio_get_level(H9));
    h[7]  = gray_is_active(gpio_get_level(H10));
    h[8] = gray_is_active(gpio_get_level(H11));
    h[9] = gray_is_active(gpio_get_level(H12));

    gray_value = 0;
    gray_value |= h[0]  << 9;
    gray_value |= h[1]  << 8;
    gray_value |= h[2]  << 7;
    gray_value |= h[3]  << 6;
    gray_value |= h[4]  << 5;
    gray_value |= h[5]  << 4;
    gray_value |= h[6]  << 3;
    gray_value |= h[7]  << 2;
    gray_value |= h[8] << 1;
    gray_value |= h[9] << 0;

    return gray_value;
}

/*
 * 计算巡线偏差
 *
 * H1  最左，权重 -11
 * H2        -9
 * H3        -7
 * H4        -5
 * H5        -3
 * H6        -1
 * H7        +1
 * H8        +3
 * H9        +5
 * H10       +7
 * H11       +9
 * H12 最右，权重 +11
 *
 * error < 0：黑线在左边，小车应该左转
 * error > 0：黑线在右边，小车应该右转
 * error = 0：黑线在中间
 */
int16 Xunji_Get_Error(void)
{
    uint8 h[10];
    int16 weight[10] = {-13, -11, -9, -7, -2, 1, 5, 7, 9, 11};

    int16 sum = 0;
    int16 count = 0;
    uint8 i;

	for(i = 0; i < 10; i++)
	{
		h[i] = (gray_value >> (9 - i)) & 0x01;
	}
	

    for(i = 0; i < 10; i++)
    {
        if(h[i])
        {
            sum += weight[i];
            count++;
        }
    }

    /*
     * count >= 10 表示全部没有检测到黑线，也就是丢线
     */
    if(count >= 10)
    {
	#if LOST_USE_LAST_DIR
		/*
		 * 全亮时，说明进入弯道/大黑区/丢线状态
		 * 不要固定写 -2，要沿着上一次误差方向继续转
		 */
		if(last_error > 0)
		{
			xunji_error = -13;
		}
		else if(last_error < 0)
		{
			xunji_error = 13;
		}
		else
		{
			xunji_error = 0;
		}
	#else
		xunji_error = 0;
	#endif
		}
	else if(count == 0)
	{
		xunji_error = 0;
	}
	else
	{
		xunji_error = sum / count;
		
	}

	return xunji_error;
}


/*
 * 巡线 PID
 */
int16 Xunji_PID_Calc(float error)
{
    float pid_out;

    integral_error += error;

    /*
     * 积分限幅，防止积分越积越大
     * 目前 KI = 0，其实不会起作用，但是保留这个更安全
     */
    if(integral_error > 100)  integral_error = 100;
    if(integral_error < -100) integral_error = -100;

    pid_out = XUNJI_KP * error
            + XUNJI_KI * integral_error
            + XUNJI_KD * (error - last_error);

    last_error = error;
	
	//丢线速度过大问题
	if(pid_out > PID_OUT_MAX)  pid_out = PID_OUT_MAX;
	if(pid_out < -PID_OUT_MAX) pid_out = -PID_OUT_MAX;
	
    xunji_pid_out = (int16)pid_out;

    return xunji_pid_out;
}


/*
 * 巡线主控制函数
 * 建议放在 while(1) 里面周期调用
 */
void Xunji_Control(void)
{
#if 0
	int16 error;
	int16 pid;
	int16 left_speed;
	int16 right_speed;

	Gray_Read();

	error = Xunji_Get_Error();
	pid = Xunji_PID_Calc(error);

	/*

	差速控制

	error < 0：线在左边，pid < 0

	left_speed 变小，right_speed 变大，小车左转*/
	left_speed  = LEFT_BASE_SPEED - pid;
	right_speed = RIGHT_BASE_SPEED + pid;

	left_speed  = limit_int16(left_speed,  MIN_SPEED, MAX_SPEED);
	right_speed = limit_int16(right_speed, MIN_SPEED, MAX_SPEED);

	/*

	这里根据你的电机函数改
	如果你 PWM.c 里面函数名不是 Motor_LeftSetSpeed / Motor_RightSetSpeed，
	就把下面两句换成你自己的函数。*/
	Motor_LeftSetSpeed(MOTOR_FORWARD,left_speed);
	Motor_RightSetSpeed(MOTOR_FORWARD,right_speed);
	#endif
#if 1
    int16 error;
    int16 pid;
    int16 left_speed;
    int16 right_speed;

    Gray_Read();

    error = Xunji_Get_Error();

    /*
     * 直接用 error 判断大弯
     * error <= -7：线明显在左边，左转
     * error >=  7：线明显在右边，右转
     */
    if(error <= -8)
    {
        Motor_LeftSetSpeed(MOTOR_FORWARD, 1700);
        Motor_RightSetSpeed(MOTOR_FORWARD, 500);
		system_delay_ms(50);
        return;
    }
    else
    {
        /*
		 * 小偏差继续 PID 差速
		 */
		pid = Xunji_PID_Calc(error);

		left_speed  = LEFT_BASE_SPEED - pid;
		right_speed = RIGHT_BASE_SPEED + pid;

		left_speed  = limit_int16(left_speed,  MIN_SPEED, MAX_SPEED);
		right_speed = limit_int16(right_speed, MIN_SPEED, MAX_SPEED);

		Motor_LeftSetSpeed(MOTOR_FORWARD, left_speed);
		Motor_RightSetSpeed(MOTOR_FORWARD, right_speed);
    }

    /*
     * 小偏差继续 PID 差速
     */
	/*
    pid = Xunji_PID_Calc(error);

    left_speed  = LEFT_BASE_SPEED - pid;
    right_speed = RIGHT_BASE_SPEED + pid;

    left_speed  = limit_int16(left_speed,  MIN_SPEED, MAX_SPEED);
    right_speed = limit_int16(right_speed, MIN_SPEED, MAX_SPEED);

    Motor_LeftSetSpeed(MOTOR_FORWARD, left_speed);
    Motor_RightSetSpeed(MOTOR_FORWARD, right_speed);
	*/
	
#endif
}

/*
 * 停车
 */
void Xunji_Stop(void)
{
    motor_stop();
}

