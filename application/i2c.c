#define DEBUG_THIS_FILE   DEBUG_I2C_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "debug.h"

#include "i2c.h"
#include "gpio.h"


#define I2C_TRANSMITTER_MODE   0
#define I2C_RECEIVER_MODE      1


static uint32_t TM_I2C_Timeout;


extern void i2c_init(I2C_TypeDef *I2Cx) {
    //! Peripheral clock enable
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    //! I2C Initialization
    LL_I2C_InitTypeDef I2C_InitStruct = {0};
    LL_I2C_DisableOwnAddress2(I2Cx);
    LL_I2C_DisableGeneralCall(I2Cx);
    LL_I2C_EnableClockStretching(I2Cx);
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.ClockSpeed = SCL_CLOCK;
    I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
    I2C_InitStruct.OwnAddress1 = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(I2Cx, &I2C_InitStruct);
    LL_I2C_SetOwnAddress2(I2Cx, 0);
}


extern uint8_t i2c_full_write(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size) {
    // LL_I2C_Enable(I2Cx);
    CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);

    //! Start condition
    LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
    LL_I2C_GenerateStartCondition(I2Cx);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_SB(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if (--TM_I2C_Timeout == 0x00) { return 0x01; }
    }

    //! Send slave address for TX
    LL_I2C_TransmitData8(I2Cx, address & ~I2C_OAR1_ADD0);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_ADDR(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if (--TM_I2C_Timeout == 0x00) { return 0x02; }
    }
    LL_I2C_ClearFlag_ADDR(I2Cx);

    //! Send memory adress
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_TXE(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if(--TM_I2C_Timeout == 0) { 
            LL_I2C_GenerateStopCondition(I2Cx);
            return 3; 
        }
    }

    LL_I2C_TransmitData8(I2Cx, mem);

    while(size) {
        TM_I2C_Timeout = TM_I2C_TIMEOUT;
        while (!LL_I2C_IsActiveFlag_TXE(I2Cx)) {
            if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
            if(--TM_I2C_Timeout == 0) { 
                LL_I2C_GenerateStopCondition(I2Cx);
                return 4; 
            }
        }

        LL_I2C_TransmitData8(I2Cx, *(buffer++));
        size--;

        if(LL_I2C_IsActiveFlag_BTF(I2Cx) && size != 0) {
            LL_I2C_TransmitData8(I2Cx, *(buffer++));
            size--;
        }

    }

    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_BTF(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if(--TM_I2C_Timeout == 0) { 
            LL_I2C_GenerateStopCondition(I2Cx);
            return 5; 
        }
    }

    LL_I2C_GenerateStopCondition(I2Cx);
    return 0;
}


extern uint8_t i2c_full_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size) {
    // LL_I2C_Enable(I2Cx);
    CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);

    //! Start condition
    LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
    LL_I2C_GenerateStartCondition(I2Cx);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_SB(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if (--TM_I2C_Timeout == 0x00) { return 0x01; }
    }

    //! Send slave address for TX
    LL_I2C_TransmitData8(I2Cx, address & ~I2C_OAR1_ADD0);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_ADDR(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if (--TM_I2C_Timeout == 0x00) { return 0x02; }
    }
    LL_I2C_ClearFlag_ADDR(I2Cx);

    //! Send memory adress
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_TXE(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if(--TM_I2C_Timeout == 0) { 
            LL_I2C_GenerateStopCondition(I2Cx);
            return 3; 
        }
    }
    LL_I2C_TransmitData8(I2Cx, mem);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_TXE(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if(--TM_I2C_Timeout == 0) { 
            LL_I2C_GenerateStopCondition(I2Cx);
            return 4; 
        }
    }

    //! Restart
    LL_I2C_GenerateStartCondition(I2Cx);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_SB(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if (--TM_I2C_Timeout == 0x00) { return 0x05; }
    }

    //! Send slave address for RX
    LL_I2C_TransmitData8(I2Cx, address | I2C_OAR1_ADD0);
    TM_I2C_Timeout = TM_I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_ADDR(I2Cx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if (--TM_I2C_Timeout == 0x00) { return 0x6; }
    }

    // CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);
    if(size == 0) {
        LL_I2C_ClearFlag_ADDR(I2Cx);
        LL_I2C_GenerateStopCondition(I2Cx);
    }
    else if(size == 1) {
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
        LL_I2C_ClearFlag_ADDR(I2Cx);
        LL_I2C_GenerateStopCondition(I2Cx);
    }
    else if(size == 2) {
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
        SET_BIT(I2Cx->CR1, I2C_CR1_POS);
        LL_I2C_ClearFlag_ADDR(I2Cx);
    }
    else {
        LL_I2C_ClearFlag_ADDR(I2Cx);
    }

    uint8_t data;
    while(size) {
        if(size == 1) {
            TM_I2C_Timeout = TM_I2C_TIMEOUT;
            while (!LL_I2C_IsActiveFlag_RXNE(I2Cx)) {
                if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
                if (--TM_I2C_Timeout == 0x00) { return 7; }
            }
            data = LL_I2C_ReceiveData8(I2Cx);
        }
        else if(size == 2) {
            TM_I2C_Timeout = TM_I2C_TIMEOUT;
            while (!LL_I2C_IsActiveFlag_BTF(I2Cx)) {
                if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
                if (--TM_I2C_Timeout == 0x00) { return 8; }
            }
            LL_I2C_GenerateStopCondition(I2Cx);
            data = LL_I2C_ReceiveData8(I2Cx);
            *(buffer++) = data;
            size--;
            data = LL_I2C_ReceiveData8(I2Cx);
        }
        else if(size == 3) {
            TM_I2C_Timeout = TM_I2C_TIMEOUT;
            while (!LL_I2C_IsActiveFlag_BTF(I2Cx)) {
                if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
                if (--TM_I2C_Timeout == 0x00) { return 9; }
            }
            LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
            data = LL_I2C_ReceiveData8(I2Cx);
        }
        else {
            TM_I2C_Timeout = TM_I2C_TIMEOUT;
            while (!LL_I2C_IsActiveFlag_RXNE(I2Cx)) {
                if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
                if (--TM_I2C_Timeout == 0x00) { return 10; }
            }
            data = LL_I2C_ReceiveData8(I2Cx);
            if(LL_I2C_IsActiveFlag_BTF(I2Cx)) {
                *(buffer++) = data;
                size--;
                data = LL_I2C_ReceiveData8(I2Cx);
            }
        }
        *(buffer++) = data;
        size--;
    }

    return 0;
}

//! ---- OLD -----
// extern uint8_t i2c_start(I2C_TypeDef *I2Cx, unsigned char address) {

//     LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
//     // LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
    
//     /* Generate I2C start pulse */
//     LL_I2C_GenerateStartCondition(I2Cx);
    
//     /* Wait till I2C is busy */
//     TM_I2C_Timeout = TM_I2C_TIMEOUT;
//     while (!LL_I2C_IsActiveFlag_SB(I2Cx)) {
//         if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//         if (--TM_I2C_Timeout == 0x00) {
//             printf("i2c_start timeout step 0\n");
//             TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//             return 0x01;
//         }
//     }

//     /* Send write/read bit */
//     if ((address & I2C_OAR1_ADD0) == I2C_TRANSMITTER_MODE) {
//         /* Send address with zero last bit */
//         LL_I2C_TransmitData8(I2Cx, address & ~I2C_OAR1_ADD0);

//         TM_I2C_Timeout = TM_I2C_TIMEOUT;
//         while (!LL_I2C_IsActiveFlag_ADDR(I2Cx)) {
//             if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//             if (--TM_I2C_Timeout == 0x00) {
//                 printf("i2c_start timeout step T1\n");
//                 TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//                 return 0x10;
//             }
//         }
//     }
//     if ((address & I2C_OAR1_ADD0) == I2C_RECEIVER_MODE) {
//         /* Send address with 1 last bit */
//         LL_I2C_TransmitData8(I2Cx, address | I2C_OAR1_ADD0);

//         TM_I2C_Timeout = TM_I2C_TIMEOUT;
//         while (LL_I2C_IsActiveFlag_ADDR(I2Cx)) {
//             if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//             if (--TM_I2C_Timeout == 0x00) {
//                 printf("i2c_start timeout step R1\n");
//                 TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//                 return 0x11;
//             }
//         }
//     }

//     /* Read status register to clear ADDR flag */
//     LL_I2C_ClearFlag_ADDR(I2Cx);
//     printf("s0 i2c_flags [0x%02X]\n", (uint8_t)(I2Cx->SR1 & 0xff));
    
//     return 0;
// }


// extern uint8_t i2c_rep_start(I2C_TypeDef *I2Cx, uint8_t address) {
//     return i2c_start(I2Cx, address);
// }


// extern uint8_t i2c_write(I2C_TypeDef *I2Cx, uint8_t data) {
//     /* Wait till I2C is not busy anymore */
//     TM_I2C_Timeout = TM_I2C_TIMEOUT;
//     while (!LL_I2C_IsActiveFlag_TXE(I2Cx)) {
//         if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//         if(--TM_I2C_Timeout == 0) {
//             printf("i2c_write timeout\n");
//             TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//             return 1;
//         }
//     }
    
//     /* Send I2C data */
//     LL_I2C_TransmitData8(I2Cx, data);
//     return 0;
// }


// extern uint8_t i2c_readAck(I2C_TypeDef *I2Cx, uint8_t *out) {
//     uint8_t data;
    
//     // if(LL_I2C_IsActiveFlag_ADDR(I2Cx)) { LL_I2C_ClearFlag_ADDR(I2Cx); }

//     /* Enable ACK */
//     LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
    
//     /* Wait till not received */
//     TM_I2C_Timeout = TM_I2C_TIMEOUT;
//     // while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
//     while (!LL_I2C_IsActiveFlag_BTF(I2Cx)) {
//         if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//         if (--TM_I2C_Timeout == 0x00) {
//             printf("t0 i2c_readAck timeout [0x%02X]\n", (uint8_t)(I2Cx->SR1 & 0xff));
//             TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//             return 1;
//         }
//     }
    
//     /* Read data */
//     data = LL_I2C_ReceiveData8(I2Cx);
    
//     /* Return data */
//     if(out != NULL) { *out = data; }
//     return 0;
// }


// extern uint8_t i2c_readNak(I2C_TypeDef *I2Cx, uint8_t *out) {
//     uint8_t data;

//     if(LL_I2C_IsActiveFlag_ADDR(I2Cx)) { LL_I2C_ClearFlag_ADDR(I2Cx); }
//     printf("r0 i2c_flags [0x%02X]\n", (uint8_t)(I2Cx->SR1 & 0xff));
    
//     /* Disable ACK */
//     LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
    
//     /* Generate stop */
//     LL_I2C_GenerateStopCondition(I2Cx);
    
//     /* Wait till received */
//     TM_I2C_Timeout = TM_I2C_TIMEOUT;
//     // while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
//     while (!LL_I2C_IsActiveFlag_RXNE(I2Cx) || !LL_I2C_IsActiveFlag_BTF(I2Cx)) {
//         if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//         if (--TM_I2C_Timeout == 0x00) {
//             printf("i2c_readNack timeout [0x%02X]\n", (uint8_t)(I2Cx->SR1 & 0xff));
//             TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//             return 1;
//         }
//     }

//     /* Read data */
//     data = LL_I2C_ReceiveData8(I2Cx);
    
//     /* Return data */
//     if(out != NULL) { *out = data; }
//     return 0;
// }


// extern uint8_t i2c_stop(I2C_TypeDef *I2Cx) {
//     printf("stop i2c_flags [0x%02X]\n", (uint8_t)(I2Cx->SR1 & 0xff));

//     /* Wait till transmitter not empty */
//     TM_I2C_Timeout = TM_I2C_TIMEOUT;
//     while ( !LL_I2C_IsActiveFlag_TXE(I2Cx) || !LL_I2C_IsActiveFlag_BTF(I2Cx) ) {
//         if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
//         if (--TM_I2C_Timeout == 0x00) {
//             printf("i2c_stop timeout [0x%02X]\n", (uint8_t)(I2Cx->SR1 & 0xff));
//             TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_2);
//             return 1;
//         }
//     }
    
//     /* Generate stop */
//     // I2Cx->CR1 |= I2C_CR1_STOP;
//     LL_I2C_GenerateStopCondition(I2Cx);
//     return 0;
// }


// Print out all the active I2C addresses on the bus
// extern void searchI2C(I2C_TypeDef *I2Cx) {
//     uint8_t devAdr;
//     debugf("Discovered I2C addresses: ");
//     for( devAdr=0; devAdr<=127; devAdr++ ) {
//         if( !i2c_start(I2Cx, (devAdr<<1) | I2C_WRITE ) ) {
//             debugf("%02X ", devAdr);
//         }
//         i2c_stop(I2Cx);                             // set stop conditon = release bus
//     }
//     debugf("done\n");
// }
