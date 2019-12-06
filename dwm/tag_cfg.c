/*! ------------------------------------------------------------------------------------------------------------------
 * @file    tag_cfg.c
 * @brief   Decawave device configuration and control functions
 *
 * @attention
 *
 * Copyright 2017 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#include "dwm_api.h"
#include "hal.h"
#include "nrf_delay.h"

int main(void)
{
   int i;
   int wait_period = 1000;
   dwm_cfg_tag_t cfg_tag;
   dwm_cfg_t cfg_node;

   // printf("dwm_init(): dev%d\n", HAL_DevNum());
   dwm_init();

   // printf("Setting to tag: dev%d.\n", HAL_DevNum());
   cfg_tag.low_power_en = 0;
   cfg_tag.meas_mode = DWM_MEAS_MODE_TWR;
   cfg_tag.loc_engine_en = 1;
   cfg_tag.common.led_en = 1;
   cfg_tag.common.ble_en = 1;
   cfg_tag.common.uwb_mode = DWM_UWB_MODE_ACTIVE;
   cfg_tag.common.fw_update_en = 0;
   // printf("dwm_cfg_tag_set(&cfg_tag): dev%d.\n", HAL_DevNum());
   dwm_cfg_tag_set(&cfg_tag);

   printf("Wait 2s for node to reset.\n");
   nrf_delay_ms(2000);
   dwm_cfg_get(&cfg_node);

   // printf("Comparing set vs. get: dev%d.\n", HAL_DevNum());
   if((cfg_tag.low_power_en        != cfg_node.low_power_en)
   || (cfg_tag.meas_mode           != cfg_node.meas_mode)
   || (cfg_tag.loc_engine_en       != cfg_node.loc_engine_en)
   || (cfg_tag.common.led_en       != cfg_node.common.led_en)
   || (cfg_tag.common.ble_en       != cfg_node.common.ble_en)
   || (cfg_tag.common.uwb_mode     != cfg_node.common.uwb_mode)
   || (cfg_tag.common.fw_update_en != cfg_node.common.fw_update_en))
   {
      printf("low_power_en        cfg_tag=%d : cfg_node=%d\n", cfg_tag.low_power_en,     cfg_node.low_power_en);
      printf("meas_mode           cfg_tag=%d : cfg_node=%d\n", cfg_tag.meas_mode,        cfg_node.meas_mode);
      printf("loc_engine_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.loc_engine_en,    cfg_node.loc_engine_en);
      printf("common.led_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.led_en,    cfg_node.common.led_en);
      printf("common.ble_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.ble_en,    cfg_node.common.ble_en);
      printf("common.uwb_mode     cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.uwb_mode,  cfg_node.common.uwb_mode);
      printf("common.fw_update_en cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.fw_update_en, cfg_node.common.fw_update_en);
      printf("\nConfiguration failed.\n\n");
   }
   else
   {
      printf("\nConfiguration succeeded.\n\n");
   }

   dwm_loc_data_t loc;
   dwm_pos_t pos;
   loc.p_pos = &pos;
   while(1)
   {
      printf("Wait %d ms...\n", wait_period);
      nrf_delay_ms(wait_period);

      printf("dwm_loc_get(&loc):\n");

      if(dwm_loc_get(&loc) == RV_OK)
      {
         printf("\t[%d,%d,%d,%u]\n", loc.p_pos->x, loc.p_pos->y, loc.p_pos->z,
               loc.p_pos->qf);
        printf("WAS THE TAG UP THER\n\n\n");

         for (i = 0; i < loc.anchors.dist.cnt; ++i)
         {
            printf("\t%u)", i);
            printf("0x%llx", loc.anchors.dist.addr[i]);
            if (i < loc.anchors.an_pos.cnt)
            {
               printf("[%d,%d,%d,%u]", loc.anchors.an_pos.pos[i].x,
                     loc.anchors.an_pos.pos[i].y,
                     loc.anchors.an_pos.pos[i].z,
                     loc.anchors.an_pos.pos[i].qf);
            }
            printf("=%u,%u\n", loc.anchors.dist.dist[i], loc.anchors.dist.qf[i]);
         }
      }
   }

   return 0;
}
