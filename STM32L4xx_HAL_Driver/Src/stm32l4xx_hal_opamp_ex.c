/**
  ******************************************************************************
  * @file    stm32l4xx_hal_opamp_ex.c
  * @author  MCD Application Team
  * @version V1.7.1
  * @date    21-April-2017
  * @brief   Extended OPAMP HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the operational amplifier(s)(OPAMP1, OPAMP2 etc)
  *          peripheral:
  *           + Extended Initialization and de-initialization functions
  *           + Extended Peripheral Control functions
  *
  @verbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup OPAMPEx OPAMPEx
  * @brief OPAMP Extended HAL module driver
  * @{
  */

#ifdef HAL_OPAMP_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup OPAMP_Exported_Functions OPAMP Exported Functions
  * @{
  */

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx)

/** @addtogroup OPAMPEx_Exported_Functions_Group1
  * @brief    Extended operation functions
  *
@verbatim
 ===============================================================================
              ##### Extended IO operation functions #####
 ===============================================================================
  [..]
      (+) OPAMP Self calibration.

@endverbatim
  * @{
  */

/*  2 OPAMPS available */
/*  2 OPAMPS can be calibrated in parallel */
/*  Not available on STM32L43x/STM32L44x where only one OPAMP available */

/**
  * @brief  Run the self calibration of the 2 OPAMPs in parallel.
  * @note   Trimming values (PMOS & NMOS) are updated and user trimming is
  *         enabled is calibration is successful.
  * @note   Calibration is performed in the mode specified in OPAMP init
  *         structure (mode normal or low-power). To perform calibration for
  *         both modes, repeat this function twice after OPAMP init structure
  *         accordingly updated.
  * @note   Calibration runs about 10 ms (5 dichotomy steps, repeated for P
  *         and N transistors: 10 steps with 1 ms for each step).
  * @param  hopamp1 handle
  * @param  hopamp2 handle
  * @retval HAL status
  */

HAL_StatusTypeDef HAL_OPAMPEx_SelfCalibrateAll(OPAMP_HandleTypeDef *hopamp1, OPAMP_HandleTypeDef *hopamp2)
{
  HAL_StatusTypeDef status = HAL_OK;

  uint32_t trimmingvaluen1 = 0;
  uint32_t trimmingvaluep1 = 0;
  uint32_t trimmingvaluen2 = 0;
  uint32_t trimmingvaluep2 = 0;

/* Selection of register of trimming depending on power mode: OTR or LPOTR */
  __IO uint32_t* tmp_opamp1_reg_trimming;
  __IO uint32_t* tmp_opamp2_reg_trimming;

  uint32_t delta;
  uint32_t opampmode1;
  uint32_t opampmode2;

  if((hopamp1 == NULL) || (hopamp1->State == HAL_OPAMP_STATE_BUSYLOCKED) || \
     (hopamp2 == NULL) || (hopamp2->State == HAL_OPAMP_STATE_BUSYLOCKED))
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check if OPAMP in calibration mode and calibration not yet enable */
    if((hopamp1->State ==  HAL_OPAMP_STATE_READY) && (hopamp2->State ==  HAL_OPAMP_STATE_READY))
    {
      /* Check the parameter */
      assert_param(IS_OPAMP_ALL_INSTANCE(hopamp1->Instance));
      assert_param(IS_OPAMP_ALL_INSTANCE(hopamp2->Instance));

      assert_param(IS_OPAMP_POWERMODE(hopamp1->Init.PowerMode));
      assert_param(IS_OPAMP_POWERMODE(hopamp2->Init.PowerMode));

      /* Save OPAMP mode as in                                       */
      /* STM32L471xx STM32L475xx STM32L476xx STM32L485xx STM32L486xx */
      /* the calibration is not working in PGA mode                  */
      opampmode1 = READ_BIT(hopamp1->Instance->CSR,OPAMP_CSR_OPAMODE);
      opampmode2 = READ_BIT(hopamp2->Instance->CSR,OPAMP_CSR_OPAMODE);

      /* Use of standalone mode */
      MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_OPAMODE, OPAMP_STANDALONE_MODE);
      MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_OPAMODE, OPAMP_STANDALONE_MODE);

      /*  user trimming values are used for offset calibration */
      SET_BIT(hopamp1->Instance->CSR, OPAMP_CSR_USERTRIM);
      SET_BIT(hopamp2->Instance->CSR, OPAMP_CSR_USERTRIM);

      /* Select trimming settings depending on power mode */
      if (hopamp1->Init.PowerMode == OPAMP_POWERMODE_NORMAL)
      {
        tmp_opamp1_reg_trimming = &OPAMP1->OTR;
      }
      else
      {
        tmp_opamp1_reg_trimming = &OPAMP1->LPOTR;
      }

      if (hopamp2->Init.PowerMode == OPAMP_POWERMODE_NORMAL)
      {
        tmp_opamp2_reg_trimming = &OPAMP2->OTR;
      }
      else
      {
        tmp_opamp2_reg_trimming = &OPAMP2->LPOTR;
      }

      /* Enable calibration */
      SET_BIT (hopamp1->Instance->CSR, OPAMP_CSR_CALON);
      SET_BIT (hopamp2->Instance->CSR, OPAMP_CSR_CALON);

      /* 1st calibration - N */
      CLEAR_BIT (hopamp1->Instance->CSR, OPAMP_CSR_CALSEL);
      CLEAR_BIT (hopamp2->Instance->CSR, OPAMP_CSR_CALSEL);

      /* Enable the selected opamp */
      SET_BIT (hopamp1->Instance->CSR, OPAMP_CSR_OPAMPxEN);
      SET_BIT (hopamp2->Instance->CSR, OPAMP_CSR_OPAMPxEN);

      /* Init trimming counter */
      /* Medium value */
      trimmingvaluen1 = 16;
      trimmingvaluen2 = 16;
      delta = 8;

      while (delta != 0)
      {
        /* Set candidate trimming */
        /* OPAMP_POWERMODE_NORMAL */
        MODIFY_REG(*tmp_opamp1_reg_trimming, OPAMP_OTR_TRIMOFFSETN, trimmingvaluen1);
        MODIFY_REG(*tmp_opamp2_reg_trimming, OPAMP_OTR_TRIMOFFSETN, trimmingvaluen2);

        /* OFFTRIMmax delay 1 ms as per datasheet (electrical characteristics */
        /* Offset trim time: during calibration, minimum time needed between */
        /* two steps to have 1 mV accuracy */
        HAL_Delay(OPAMP_TRIMMING_DELAY);

        if (READ_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALOUT) != RESET)
        {
          /* OPAMP_CSR_CALOUT is HIGH try lower trimming */
          trimmingvaluen1 -= delta;
        }
        else
        {
          /* OPAMP_CSR_CALOUT is LOW try higher trimming */
          trimmingvaluen1 += delta;
        }

        if (READ_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALOUT) != RESET)
        {
          /* OPAMP_CSR_CALOUT is HIGH try lower trimming */
          trimmingvaluen2 -= delta;
        }
        else
        {
          /* OPAMP_CSR_CALOUT is LOW try higher trimming */
          trimmingvaluen2 += delta;
        }
        /* Divide range by 2 to continue dichotomy sweep */
        delta >>= 1;
      }

      /* Still need to check if right calibration is current value or one step below */
      /* Indeed the first value that causes the OUTCAL bit to change from 0 to 1  */
      /* Set candidate trimming */
      MODIFY_REG(*tmp_opamp1_reg_trimming, OPAMP_OTR_TRIMOFFSETN, trimmingvaluen1);
      MODIFY_REG(*tmp_opamp2_reg_trimming, OPAMP_OTR_TRIMOFFSETN, trimmingvaluen2);

      /* OFFTRIMmax delay 1 ms as per datasheet (electrical characteristics */
      /* Offset trim time: during calibration, minimum time needed between */
      /* two steps to have 1 mV accuracy */
      HAL_Delay(OPAMP_TRIMMING_DELAY);

      if ((READ_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALOUT)) == 0)
      {
        /* Trimming value is actually one value more */
        trimmingvaluen1++;
        MODIFY_REG(*tmp_opamp1_reg_trimming, OPAMP_OTR_TRIMOFFSETN, trimmingvaluen1);
      }

      if ((READ_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALOUT)) == 0)
      {
        /* Trimming value is actually one value more */
        trimmingvaluen2++;
        MODIFY_REG(*tmp_opamp2_reg_trimming, OPAMP_OTR_TRIMOFFSETN, trimmingvaluen2);
      }

      /* 2nd calibration - P */
      SET_BIT (hopamp1->Instance->CSR, OPAMP_CSR_CALSEL);
      SET_BIT (hopamp2->Instance->CSR, OPAMP_CSR_CALSEL);

      /* Init trimming counter */
      /* Medium value */
      trimmingvaluep1 = 16;
      trimmingvaluep2 = 16;
      delta = 8;

      while (delta != 0)
      {
        /* Set candidate trimming */
        /* OPAMP_POWERMODE_NORMAL */
        MODIFY_REG(*tmp_opamp1_reg_trimming, OPAMP_OTR_TRIMOFFSETP, (trimmingvaluep1<<OPAMP_INPUT_NONINVERTING));
        MODIFY_REG(*tmp_opamp2_reg_trimming, OPAMP_OTR_TRIMOFFSETP, (trimmingvaluep2<<OPAMP_INPUT_NONINVERTING));

        /* OFFTRIMmax delay 1 ms as per datasheet (electrical characteristics */
        /* Offset trim time: during calibration, minimum time needed between */
        /* two steps to have 1 mV accuracy */
        HAL_Delay(OPAMP_TRIMMING_DELAY);

        if (READ_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALOUT) != RESET)
        {
          /* OPAMP_CSR_CALOUT is HIGH try higher trimming */
          trimmingvaluep1 += delta;
        }
        else
        {
          /* OPAMP_CSR_CALOUT is HIGH try lower trimming */
          trimmingvaluep1 -= delta;
        }

        if (READ_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALOUT) != RESET)
        {
          /* OPAMP_CSR_CALOUT is HIGH try higher trimming */
          trimmingvaluep2 += delta;
        }
        else
        {
          /* OPAMP_CSR_CALOUT is LOW try lower trimming */
          trimmingvaluep2 -= delta;
        }
        /* Divide range by 2 to continue dichotomy sweep */
        delta >>= 1;
      }

      /* Still need to check if right calibration is current value or one step below */
      /* Indeed the first value that causes the OUTCAL bit to change from 1 to 0  */
      /* Set candidate trimming */
      MODIFY_REG(*tmp_opamp1_reg_trimming, OPAMP_OTR_TRIMOFFSETP, (trimmingvaluep1<<OPAMP_INPUT_NONINVERTING));
      MODIFY_REG(*tmp_opamp2_reg_trimming, OPAMP_OTR_TRIMOFFSETP, (trimmingvaluep2<<OPAMP_INPUT_NONINVERTING));

      /* OFFTRIMmax delay 1 ms as per datasheet (electrical characteristics */
      /* Offset trim time: during calibration, minimum time needed between */
      /* two steps to have 1 mV accuracy */
      HAL_Delay(OPAMP_TRIMMING_DELAY);

      if (READ_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALOUT) != RESET)
      {
        /* Trimming value is actually one value more */
        trimmingvaluep1++;
        MODIFY_REG(*tmp_opamp1_reg_trimming, OPAMP_OTR_TRIMOFFSETP, (trimmingvaluep1<<OPAMP_INPUT_NONINVERTING));
      }

      if (READ_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALOUT) != RESET)
      {
        /* Trimming value is actually one value more */
        trimmingvaluep2++;
        MODIFY_REG(*tmp_opamp2_reg_trimming, OPAMP_OTR_TRIMOFFSETP, (trimmingvaluep2<<OPAMP_INPUT_NONINVERTING));
      }

      /* Disable the OPAMPs */
      CLEAR_BIT (hopamp1->Instance->CSR, OPAMP_CSR_OPAMPxEN);
      CLEAR_BIT (hopamp2->Instance->CSR, OPAMP_CSR_OPAMPxEN);

      /* Disable calibration & set normal mode (operating mode) */
      CLEAR_BIT (hopamp1->Instance->CSR, OPAMP_CSR_CALON);
      CLEAR_BIT (hopamp2->Instance->CSR, OPAMP_CSR_CALON);

      /* Self calibration is successful */
      /* Store calibration (user trimming) results in init structure. */

      /* Set user trimming mode */
      hopamp1->Init.UserTrimming = OPAMP_TRIMMING_USER;
      hopamp2->Init.UserTrimming = OPAMP_TRIMMING_USER;

      /* Affect calibration parameters depending on mode normal/low power */
      if (hopamp1->Init.PowerMode != OPAMP_POWERMODE_LOWPOWER)
      {
        /* Write calibration result N */
        hopamp1->Init.TrimmingValueN = trimmingvaluen1;
        /* Write calibration result P */
        hopamp1->Init.TrimmingValueP = trimmingvaluep1;
      }
      else
      {
        /* Write calibration result N */
        hopamp1->Init.TrimmingValueNLowPower = trimmingvaluen1;
        /* Write calibration result P */
        hopamp1->Init.TrimmingValuePLowPower = trimmingvaluep1;
      }

      if (hopamp2->Init.PowerMode != OPAMP_POWERMODE_LOWPOWER)
      {
        /* Write calibration result N */
        hopamp2->Init.TrimmingValueN = trimmingvaluen2;
        /* Write calibration result P */
        hopamp2->Init.TrimmingValueP = trimmingvaluep2;
      }
      else
      {
        /* Write calibration result N */
        hopamp2->Init.TrimmingValueNLowPower = trimmingvaluen2;
        /* Write calibration result P */
        hopamp2->Init.TrimmingValuePLowPower = trimmingvaluep2;
      }

      /* Update OPAMP state */
      hopamp1->State = HAL_OPAMP_STATE_READY;
      hopamp2->State = HAL_OPAMP_STATE_READY;

      /* Restore OPAMP mode after calibration */
      MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_OPAMODE, opampmode1);
      MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_OPAMODE, opampmode2);
    }
    else
    {
      /* At least one OPAMP can not be calibrated */
      status = HAL_ERROR;
    }
  }
  return status;
}

/**
  * @}
  */

#endif

/** @defgroup OPAMPEx_Exported_Functions_Group2 Peripheral Control functions
 *  @brief   Peripheral Control functions
 *
@verbatim
 ===============================================================================
             ##### Peripheral Control functions #####
 ===============================================================================
    [..]
      (+) OPAMP unlock.

@endverbatim
  * @{
  */

/**
  * @brief  Unlock the selected OPAMP configuration.
  * @note   This function must be called only when OPAMP is in state "locked".
  * @param  hopamp: OPAMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPAMPEx_Unlock(OPAMP_HandleTypeDef* hopamp)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the OPAMP handle allocation */
  /* Check if OPAMP locked */
  if((hopamp == NULL) || (hopamp->State == HAL_OPAMP_STATE_RESET)
                      || (hopamp->State == HAL_OPAMP_STATE_READY)
                      || (hopamp->State == HAL_OPAMP_STATE_CALIBBUSY)
                      || (hopamp->State == HAL_OPAMP_STATE_BUSY))

  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));

   /* OPAMP state changed to locked */
    hopamp->State = HAL_OPAMP_STATE_BUSY;
  }
  return status;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_OPAMP_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/