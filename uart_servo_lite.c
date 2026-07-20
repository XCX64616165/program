/**
JOHO UART舵机操作_发送与接收
轻SDK
HAL Edition
Add SyncWrite 2025.04 by Gem
***/
#include "uart_servo_lite.h"


// 数据帧转换为字节数组并发送
void JOHO_Package2Send(PackageTypeDef *pkg,  UART_HandleTypeDef *usart){
    uint8_t checksum; // 校验和
		uint8_t byteArray[pkg->size+4];   //后续需要可以使用环形缓冲代替
	
//    // 写入帧头 不参与校验，在计算校验时体现
//    RingBuffer_WriteUShort(ringBuf, pkg->header);
  uint8_t *p= (uint8_t *)&pkg->header;
	byteArray[0]=p[0];byteArray[1]=p[1];

  // 写入舵机ID
	byteArray[2]=pkg->usId;
//    RingBuffer_WriteByte(ringBuf, pkg->usId);
    // 写入包的长度
	byteArray[3]= pkg->size;
//    RingBuffer_WriteByte(ringBuf, pkg->size);
	//写入状态（or 指令类型）
	byteArray[4]=pkg->sstat;
//	RingBuffer_WriteByte(ringBuf, pkg->sstat);
    // 写入内容主题
	if(pkg->size != 2){
		for(uint16_t idx=0; idx<pkg->size-2; idx++){
			byteArray[5+idx]=pkg->content[idx];
	}	
	}
//    RingBuffer_WriteByteArray(ringBuf, pkg->content, pkg->size-2);
    // 计算校验和
    checksum = Array_GetChecksum(byteArray, pkg->size);
    // 写入校验和
    byteArray[pkg->size+3] = checksum;
	
	//发送字符串
	HAL_UART_Transmit(usart,byteArray,pkg->size+4,HAL_MAX_DELAY);
	
}

uint8_t Array_GetChecksum(uint8_t byteArray[],uint8_t size){
uint8_t bSum=0;

	for(int i=2; i < size+3; i++){
		// 指针逐次右移
		bSum = (bSum + byteArray[i]);		
	}
	bSum = ~(bSum)&0xFF;
	//计算校验和
	return bSum;
}


//构造并发送需要的数据帧 【字头	ID号	数据长度	指令类型	参数	数据校验】
void JOHO_PackageBuild_Send(UART_HandleTypeDef *usart, uint8_t usId, uint8_t size,uint8_t cmdType, uint8_t *content){
  // 可以通过申请内存，获得动态数据包大小，此处数据包大小固定
    PackageTypeDef pkg;
	
    // 设置帧头
    pkg.header = JOHO_PACK_REQUEST_HEADER;
    // 设置ID号
    pkg.usId = usId;
    // 数据长度		
    pkg.size = size;
	
	  //指令类型
		pkg.sstat = cmdType;
		// 逐一拷贝数组里面的内容
		for(int i=0; i<size-2; i++){
			pkg.content[i] = content[i];
		}
//    // 将pkg发送到发送缓冲区sendBuf里面
//    JOHO_Package2RingBuffer(&pkg, usart->sendBuf);
//		// 通过串口将数据发送出去
//    Usart_SendAll(usart);
		JOHO_Package2Send(&pkg,usart);
}




// 发送数据帧-HEX
void USL_Send_HEX(UART_HandleTypeDef *usart, uint8_t size, uint8_t *content){

		//发送字符串
	HAL_UART_Transmit(usart,content,size,HAL_MAX_DELAY);
//	RingBuffer_WriteByteArray( usart->sendBuf,content,size);
//	
//	// 通过串口将数据发送出去
//    Usart_SendAll(usart);
}


//接收数据帧
JOHO_STATUS USL_RecvPackage(UART_HandleTypeDef *usart,PackageTypeDef *pkg){
	
		pkg->status = 0; // Package状态初始化	
    uint8_t bIdx = 0; // 接收的参数字节索引号
    uint16_t header = 0; // 帧头
	 uint8_t rec;
	
		// 倒计时开始,100
//    SysTick_CountdownBegin(JOHO_TIMEOUT_MS);
	
	uint32_t lastTick = HAL_GetTick(); // 获取当前滴答定时器的计数值
	
	
		// 如果没有超时
    while ((HAL_GetTick() - lastTick) <= JOHO_TIMEOUT_MS){
//			if (RingBuffer_GetByteUsed(usart->recvBuf) == 0){
//			// 没有新的字节读进来, 继续等待
//            continue;
//        }
if(HAL_UART_Receive(usart, &rec, 1, 10) != HAL_OK){
continue;
}
        if(pkg->status == JOHO_RECV_FLAG_CONTENT){
            // 参数内容接收完成
            // 接收校验码
            pkg->checksum = rec;
            // 设置状态码-校验码已经接收
            pkg->status = pkg->status | JOHO_RECV_FLAG_CHECKSUM;
            // 直接进行校验码检查
            if (JOHO_CalcChecksum(pkg) != pkg->checksum){
//                // 撤销倒计时
//                SysTick_CountdownCancel();
                // 校验码错误
                return JOHO_STATUS_CHECKSUM_ERROR;
            }else{
//                // 撤销倒计时
//                SysTick_CountdownCancel();
                // 数据帧接收成功
                return JOHO_STATUS_SUCCESS;
            }
        }else if(pkg->status == JOHO_RECV_FLAG_SSTAT){
            // 状态已经接收完成
            // 接收参数字节
				
            pkg->content[bIdx] = rec;
            bIdx ++;
            // 判断是否接收完成
            if (bIdx == (pkg->size-2)){
                // 标记为参数接收完成
                pkg->status = JOHO_RECV_FLAG_CONTENT;
            }
        }
				else if(pkg->status == JOHO_RECV_FLAG_SIZE){
				// Size已经接收完成
					//接收状态
					pkg->sstat = rec;
					pkg->status = JOHO_RECV_FLAG_SSTAT;

					if((pkg->size-2)==0)
					{// 标记为参数接收完成
                pkg->status = JOHO_RECV_FLAG_CONTENT;}
				}
				else if(pkg->status == JOHO_RECV_FLAG_US_ID){
            // 舵机ID接收完成
            // 接收尺寸信息
            pkg->size = rec;
            // 判断长度size是否合法
            // 参数的size大于JOHO_PACK_RESPONSE_MAX_SIZE里面的限制
            if (pkg->size > (JOHO_PACK_RESPONSE_MAX_SIZE - 5)){
//                // 撤销倒计时
//                SysTick_CountdownCancel();
                return JOHO_STATUS_SIZE_TOO_BIG;
            }
            // 设置尺寸接收完成的标志位
            pkg->status = JOHO_RECV_FLAG_SIZE;
        }else if(pkg->status == JOHO_RECV_FLAG_HEADER){
            // 帧头已接收 
            // 接收舵机ID
            pkg->usId = rec;
            // 判断指令是否合法
            // 判断控制指令是否有效 指令范围超出
            if (pkg->usId > JOHO_US_NUM){
//                // 撤销倒计时
//                SysTick_CountdownCancel();
                return JOHO_STATUS_UNKOWN_US_ID;
            }
            // 设置usId已经接收到标志位
            pkg->status = JOHO_RECV_FLAG_US_ID;
        }else{
            // 接收帧头
            if (header == 0){
                // 接收第一个字节
                header = rec;

                // 判断接收的第一个字节是否正确
                if (header != (JOHO_PACK_RESPONSE_HEADER&0xff)){
										// 如果第一个字节错误 header重置为0
                    header = 0;
                }
            }else if(header == (JOHO_PACK_RESPONSE_HEADER&0xFF)){
                // 接收帧头第二个字节
                header =  header | (rec << 8);
								// 检查第二个字节是否正确
                if(header != JOHO_PACK_RESPONSE_HEADER){
                    header = 0;
                }else{
                    pkg->header = header;
                    // 帧头接收成功
                    pkg->status = JOHO_RECV_FLAG_HEADER; 
                }
            }else{
                header = 0;
            }
        }
			
		}
	// 等待超时
    return JOHO_STATUS_TIMEOUT;

}


//// 数据帧转换为字节数组
//void JOHO_Package2RingBuffer(PackageTypeDef *pkg,  RingBufferTypeDef *ringBuf){
//    uint8_t checksum; // 校验和
////    // 写入帧头 不参与校验，在计算校验时体现
//    RingBuffer_WriteUShort(ringBuf, pkg->header);
//    // 写入舵机ID
//    RingBuffer_WriteByte(ringBuf, pkg->usId);
//    // 写入包的长度
//    RingBuffer_WriteByte(ringBuf, pkg->size);
//	//写入状态（or 指令类型）
//	RingBuffer_WriteByte(ringBuf, pkg->sstat);
//    // 写入内容主题
//	if(pkg->size != 2)
//    RingBuffer_WriteByteArray(ringBuf, pkg->content, pkg->size-2);
//    // 计算校验和
//    checksum = RingBuffer_GetChecksum(ringBuf);
//    // 写入校验和
//    RingBuffer_WriteByte(ringBuf, checksum);
//	
////	
////	printf("Auto get ServoID usId %d \r\n", pkg->usId);
////	printf("Auto get ServoID pkg->size %d \r\n", pkg->size);
////	printf("Auto get ServoID pkg->sstat %d \r\n", pkg->sstat);
////	printf("Auto get ServoID pkg->content %d \r\n", pkg->content[0]);
////	printf("Auto get ServoID checksum %d \r\n", checksum);

//}



// 计算Package的校验和
uint8_t JOHO_CalcChecksum(PackageTypeDef *pkg){
    uint8_t checksum;
		uint8_t byteArray[pkg->size+4];   //后续需要可以使用环形缓冲代替
	

  uint8_t *p= (uint8_t *)&pkg->header;
	byteArray[0]=p[0];byteArray[1]=p[1];

  // 写入舵机ID
	byteArray[2]=pkg->usId;

    // 写入包的长度
	byteArray[3]= pkg->size;

	//写入状态（or 指令类型）
	byteArray[4]=pkg->sstat;

    // 写入内容
	if(pkg->size != 2){
		for(uint16_t idx=0; idx<pkg->size-2; idx++){
			byteArray[5+idx]=pkg->content[idx];
	}	
	}

    // 计算校验和
    checksum = Array_GetChecksum(byteArray, pkg->size);
    return checksum;
}



/**
发送内容SDK
**/

//Ping 舵机 状态查询
JOHO_STATUS US_Ping(UART_HandleTypeDef *usart, uint8_t servo_id){
	uint8_t statusCode; // 状态码
	uint8_t ehcoServoId; // PING得到的舵机ID
//	printf("[PING]Send Ping Package\r\n");
	// 发送请求包
	JOHO_PackageBuild_Send(usart, servo_id, 2,CMDType_Ping, NULL);
	// 接收返回的Ping
	PackageTypeDef pkg;
	statusCode = USL_RecvPackage(usart, &pkg);
	if(statusCode == JOHO_STATUS_SUCCESS){
		// 进一步检查ID号是否匹配
		ehcoServoId = (uint8_t)pkg.usId;
		
		if (ehcoServoId != servo_id){
			// 反馈得到的舵机ID号不匹配
			return JOHO_STATUS_ID_NOT_MATCH;
		}
//		printf("[succ]Auto get ServoID %d \r\n", ehcoServoId);
	}
	return statusCode;
}


//控制舵机变换位置
void USL_SetServoAngle(UART_HandleTypeDef *usart, uint8_t servo_id, \
				float posi, uint16_t interval){
//数据校验
if(posi > 4095)posi = 4095;
if(posi <0 )		posi  = 0;			

uint16_t posit = posi;
//uint16_t time;			
uint8_t content[5];
					//例子，舵机1，-180度 FF FF 01 07 03 2A 00 00 03 E8 DF
content[0] =0x2A;
content[1] = posit	>> 8&0XFF;			
content[2] = posit	&0XFF;						
content[3] = interval	>> 8&0XFF;			
content[4] = interval	&0XFF;	
	/// 发送请求包
	JOHO_PackageBuild_Send(usart, servo_id, 7,CMDType_Write,content);
	

}

//角度查询  返回值为0-4096 为正常值
uint16_t USL_GETPositionVal(UART_HandleTypeDef *usart, uint8_t servo_id){
uint16_t value = 0xffff; 
	uint8_t statusCode;// 状态码
	uint8_t content[2];
					//舵机1，-180度 FF FF 01 07 03 2A 00 00 03 E8 DF
	content[0] =0x38;
	content[1] =0x02;
	/// 发送请求包
	JOHO_PackageBuild_Send(usart, servo_id, 4,CMDType_Read,content);
	// 接收返回的pkg
	PackageTypeDef pkg;
	statusCode = USL_RecvPackage(usart, &pkg);
	if(statusCode==0){
//			printf("[succ]position %d \r\n", pkg.content[1]|(pkg.content[0]<<8));
			value = pkg.content[1]|(pkg.content[0]<<8);
	}
	else {value = value - statusCode;}
	return value;
}

//控制扭矩开关 0关闭 1打开
void SET_Torque(UART_HandleTypeDef *usart, uint8_t servo_id,uint8_t isopen){
//	uint8_t statusCode; // 状态码
	uint8_t content[2];
	content[0] =0x28;content[1] =0x01;
	if(isopen == 0){content[1] =0x00;}
	// 发送请求包-例子：FF FF 01 04 03 28 01 CE
	JOHO_PackageBuild_Send(usart, servo_id, 4,CMDType_Write, content);

}

//同步写指令 同时控制两个舵机执行角度 SYNC WRITE
void USL_SetAngleSyncTwo(UART_HandleTypeDef *usart, uint8_t servo_id1,\
				float posi1, uint16_t interval1,uint8_t servo_id2, float posi2, uint16_t interval2){

//数据校验
if(posi1 > 4095)posi1 = 4095;
if(posi1 <0 )		posi1  = 0;
if(posi2 > 4095)posi2 = 4095;
if(posi2 <0 )		posi2  = 0;						

				
uint16_t posit1 = posi1;
uint16_t posit2 = posi2;
//uint16_t time;			
uint8_t content[12];
					//例子，0xFF 0xFF	0xFE	0x0E	0x83	
					//0x2A 0x04
					//0x01 0x07 0xD0 0x03 0xE8
					//0x03 0x07 0xD0 0x03 0xE8	0x8B
content[0] =0x2A;content[1]=0x04;
content[2] = servo_id1;				
content[3] = posit1	>> 8&0XFF;			
content[4] = posit1	&0XFF;						
content[5] = interval1	>> 8&0XFF;			
content[6] = interval1	&0XFF;
content[7] = servo_id2;						
content[8] = posit2	>> 8&0XFF;			
content[9] = posit2	&0XFF;						
content[10] = interval2	>> 8&0XFF;			
content[11] = interval2	&0XFF;					
	/// 发送请求包
	JOHO_PackageBuild_Send(usart, 0xFE, 14,CMDType_SyncWrite,content);
	

}

