#include "dma.h"
#include "bsp.h"

void MX_DMA_Init(void) {
    /* Init with LL driver */
    /* DMA controller clock enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    /* DMA interrupt init */
    /* DMA2_Stream6_IRQn interrupt configuration */
    NVIC_SetPriority(DMA2_Stream6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
    NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}
