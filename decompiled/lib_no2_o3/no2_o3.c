/* Program: no2_o3.c.o */
/* Language: AARCH64:LE:64:v8A */

/* Function: init_no2_o3_internal @ 00100000 */

undefined8 init_no2_o3_internal(undefined4 *param_1)

{
  undefined8 uVar1;
  undefined8 uVar2;
  
  *param_1 = 0;
  uVar2 = DAT_00100d08;
  uVar1 = LONGTERMVAR_START;
  *(undefined8 *)(param_1 + 1) = 0;
  *(undefined8 *)(param_1 + 3) = 0;
  *(undefined8 *)(param_1 + 5) = 0;
  *(undefined8 *)(param_1 + 7) = 0;
  *(undefined8 *)(param_1 + 9) = uVar1;
  *(undefined8 *)(param_1 + 0xb) = uVar2;
  param_1[0xd] = 0;
  param_1[0xe] = 0;
  param_1[0xf] = 0;
  param_1[0x10] = 0;
  param_1[0x11] = 0;
  return 0;
}



/* Function: init_no2_o3 @ 00100040 */

undefined8 init_no2_o3(undefined4 *param_1)

{
  undefined8 uVar1;
  undefined8 uVar2;
  
  *param_1 = 0;
  uVar2 = DAT_00100d08;
  uVar1 = LONGTERMVAR_START;
  *(undefined8 *)(param_1 + 1) = 0;
  *(undefined8 *)(param_1 + 3) = 0;
  *(undefined8 *)(param_1 + 5) = 0;
  *(undefined8 *)(param_1 + 7) = 0;
  *(undefined8 *)(param_1 + 9) = uVar1;
  *(undefined8 *)(param_1 + 0xb) = uVar2;
  param_1[0xd] = 0;
  param_1[0xe] = 0;
  param_1[0xf] = 0;
  param_1[0x10] = 0;
  param_1[0x11] = 0;
  return 0;
}



/* Function: calc_no2_o3_internal @ 00100080 */

undefined4
calc_no2_o3_internal
          (float param_1,float param_2,float param_3,float param_4,float param_5,float *param_6,
          float *param_7,uint param_8,long param_9)

{
  float *pfVar1;
  float fVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  int iVar5;
  long lVar6;
  float *pfVar7;
  float fVar8;
  float fVar9;
  undefined8 uVar10;
  double dVar11;
  double dVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  undefined1 auVar16 [16];
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float local_58;
  float fStack_54;
  undefined8 local_50;
  undefined8 uStack_48;
  float local_40;
  undefined4 uStack_3c;
  float local_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  float fStack_2c;
  undefined4 uStack_28;
  undefined4 uStack_24;
  float fStack_20;
  undefined4 uStack_1c;
  undefined4 uStack_18;
  float fStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  
  fVar9 = *param_6;
  param_8 = param_8 & 0xff;
  if (fVar9 != -NAN) {
    fVar9 = (float)((int)fVar9 + 1);
    *param_6 = fVar9;
  }
  if ((param_5 <= DAT_001009a0) || (DAT_001009a4 <= param_5)) {
    *(float *)(param_9 + 0x10) = ((param_1 / param_2 - 1.0) / DAT_001009a8 + param_3) - DAT_001009ac
    ;
    if ((uint)fVar9 < 0x33) goto LAB_001000f4;
LAB_001003e0:
    dVar11 = DAT_001009b0;
    lVar6 = 0;
    pfVar7 = param_6;
    do {
      pfVar7 = pfVar7 + 1;
      pfVar1 = (float *)((long)param_7 + lVar6);
      lVar6 = lVar6 + 4;
      dVar12 = pow((double)(*pfVar1 / *pfVar7),dVar11);
      *pfVar7 = *pfVar7 * (float)dVar12;
    } while (lVar6 != 0x10);
  }
  else {
    *(float *)(param_9 + 0x10) = param_5;
    if (0x32 < (uint)fVar9) goto LAB_001003e0;
LAB_001000f4:
    if (param_6 + 1 < param_7 + 4 && param_7 < param_6 + 5) {
      param_6[1] = *param_7;
      param_6[2] = param_7[1];
      param_6[3] = param_7[2];
      param_6[4] = param_7[3];
    }
    else {
      uVar10 = *(undefined8 *)param_7;
      *(undefined8 *)(param_6 + 3) = *(undefined8 *)(param_7 + 2);
      *(undefined8 *)(param_6 + 1) = uVar10;
    }
  }
  lVar6 = 0;
  pfVar7 = param_6;
  do {
    pfVar7 = pfVar7 + 1;
    dVar11 = log10((double)*pfVar7);
    *(float *)((long)&local_50 + lVar6) = (float)dVar11;
    lVar6 = lVar6 + 4;
  } while (lVar6 != 0x10);
  fVar9 = *param_6;
  auVar16 = NEON_fmov(0x3f800000,4);
  fVar13 = param_4 - DAT_001009b8;
  fVar8 = *(float *)(param_9 + 0x10) - 20.0;
  fVar18 = (auVar16._0_4_ + (p_10._0_4_ + fVar8 * p_20._0_4_) * fVar8 +
           (p_01._0_4_ + fVar13 * p_02._0_4_ + p_11._0_4_ * fVar8) * fVar13) * (float)local_50;
  fVar19 = (auVar16._4_4_ + (p_10._4_4_ + fVar8 * p_20._4_4_) * fVar8 +
           (p_01._4_4_ + fVar13 * p_02._4_4_ + p_11._4_4_ * fVar8) * fVar13) *
           (float)((ulong)local_50 >> 0x20);
  local_50 = CONCAT44(fVar19,fVar18);
  fVar20 = (auVar16._8_4_ + (p_10._8_4_ + fVar8 * p_20._8_4_) * fVar8 +
           (p_01._8_4_ + fVar13 * p_02._8_4_ + p_11._8_4_ * fVar8) * fVar13) * (float)uStack_48;
  fVar8 = (auVar16._12_4_ + (p_10._12_4_ + fVar8 * p_20._12_4_) * fVar8 +
          (p_01._12_4_ + fVar13 * p_02._12_4_ + p_11._12_4_ * fVar8) * fVar13) *
          (float)((ulong)uStack_48 >> 0x20);
  uStack_48 = CONCAT44(fVar8,fVar20);
  if ((uint)fVar9 < 0x33) {
    *(undefined8 *)(param_6 + 5) = local_50;
    *(undefined8 *)(param_6 + 7) = uStack_48;
LAB_001001b0:
    fVar18 = param_6[0xd];
    fVar19 = param_6[0xe];
    fVar20 = param_6[0xf];
    fVar13 = param_6[0x10];
    fVar8 = param_6[0x11];
  }
  else {
    if ((uint)fVar9 < 0x23281) {
      fVar9 = -1.0 / (float)(uint)fVar9;
      if (DAT_001009bc < fVar9) {
        if (fVar9 <= DAT_001009c0) {
          dVar11 = (double)fVar9;
        }
        else {
          dVar11 = DAT_00100988;
        }
      }
      else {
        dVar11 = DAT_00100990;
      }
    }
    else {
      dVar11 = DAT_00100980;
    }
    dVar11 = exp(dVar11);
    fVar2 = DAT_001009c4;
    fVar9 = *param_6;
    fVar17 = 1.0 - (float)dVar11;
    fVar13 = param_6[5] + ((float)local_50 - param_6[5]) * fVar17;
    fVar14 = (float)local_50 - fVar13;
    param_6[5] = fVar13;
    fVar13 = param_6[9] + fVar17 * (fVar14 * fVar14 - param_6[9]);
    fVar14 = param_6[6] + fVar17 * (local_50._4_4_ - param_6[6]);
    if (fVar13 <= fVar2) {
      fVar13 = fVar2;
    }
    fVar15 = local_50._4_4_ - fVar14;
    param_6[6] = fVar14;
    fVar2 = DAT_001009c8;
    param_6[9] = fVar13;
    fVar13 = param_6[10] + (fVar15 * fVar15 - param_6[10]) * fVar17;
    if (fVar13 <= fVar2) {
      fVar13 = fVar2;
    }
    fVar14 = param_6[7] + ((float)uStack_48 - param_6[7]) * fVar17;
    param_6[10] = fVar13;
    fVar2 = DAT_001009cc;
    fVar13 = (float)uStack_48 - fVar14;
    param_6[7] = fVar14;
    fVar13 = param_6[0xb] + (fVar13 * fVar13 - param_6[0xb]) * fVar17;
    if (fVar13 <= fVar2) {
      fVar13 = fVar2;
    }
    fVar14 = param_6[8] + (uStack_48._4_4_ - param_6[8]) * fVar17;
    param_6[0xb] = fVar13;
    fVar2 = DAT_001009d0;
    fVar13 = uStack_48._4_4_ - fVar14;
    param_6[8] = fVar14;
    fVar13 = param_6[0xc] + fVar17 * (fVar13 * fVar13 - param_6[0xc]);
    if (fVar13 <= fVar2) {
      fVar13 = fVar2;
    }
    param_6[0xc] = fVar13;
    if ((uint)fVar9 < 0x33) goto LAB_001001b0;
    uStack_3c = *(undefined4 *)(param_9 + 0x10);
    auVar16 = *(undefined1 (*) [16])(param_6 + 5);
    uStack_34 = auVar16._0_4_;
    uStack_30 = (undefined4)*(undefined8 *)(param_6 + 9);
    uStack_28 = auVar16._4_4_;
    uStack_24 = (undefined4)((ulong)*(undefined8 *)(param_6 + 9) >> 0x20);
    uStack_1c = auVar16._8_4_;
    uStack_18 = (undefined4)*(undefined8 *)(param_6 + 0xb);
    uStack_10 = auVar16._12_4_;
    uStack_c = (undefined4)((ulong)*(undefined8 *)(param_6 + 0xb) >> 0x20);
    local_40 = param_4;
    local_38 = fVar18;
    fStack_2c = fVar19;
    fStack_20 = fVar20;
    fStack_14 = fVar8;
    calc_no2_o3_O3_nnet(&local_40,&local_58);
    calc_no2_o3_NO2_nnet(&local_40,&fStack_54);
    if (param_8 >> 7 == 0) {
      dVar11 = pow(DAT_001009d8,(double)param_8);
    }
    else {
      dVar11 = pow(DAT_001009d8,(double)((float)param_8 - DAT_001009d4));
    }
    fVar8 = DAT_001009e4;
    local_58 = (float)dVar11 * local_58;
    fStack_54 = (float)dVar11 * fStack_54;
    fVar9 = *param_6;
    if (local_58 < 0.0) {
      local_58 = 0.0;
    }
    if (fStack_54 < 0.0) {
      fStack_54 = 0.0;
    }
    fVar18 = param_6[0xd] + DAT_001009e0 * (local_58 - param_6[0xd]);
    fVar19 = param_6[0xe] + DAT_001009e4 * (local_58 - param_6[0xe]);
    fVar20 = param_6[0xf] + DAT_001009e8 * (local_58 - param_6[0xf]);
    fVar13 = param_6[0x10] + DAT_001009e0 * (fStack_54 - param_6[0x10]);
    param_6[0xd] = fVar18;
    param_6[0xe] = fVar19;
    fVar8 = param_6[0x11] + fVar8 * (fStack_54 - param_6[0x11]);
    param_6[0xf] = fVar20;
    param_6[0x10] = fVar13;
    param_6[0x11] = fVar8;
  }
  fVar2 = DAT_001009ec;
  *(float *)(param_9 + 0x14) = fVar18;
  *(float *)(param_9 + 0x18) = fVar13;
  fVar14 = DAT_00100a04;
  if (fVar18 <= fVar2) {
    fVar18 = fVar18 * DAT_00100a3c;
  }
  else {
    if (fVar18 <= DAT_001009f0) {
      iVar5 = 0xa5;
      lVar6 = 2;
    }
    else if (fVar18 <= DAT_001009f4) {
      iVar5 = 0xcd;
      lVar6 = 4;
    }
    else if (fVar18 <= DAT_001009f8) {
      iVar5 = 0x195;
      lVar6 = 6;
    }
    else if (fVar18 <= DAT_001009fc) {
      iVar5 = 0x1f9;
      lVar6 = 8;
    }
    else {
      if (DAT_00100a00 < fVar18) {
        fVar18 = DAT_00100998;
        goto LAB_0010020c;
      }
      iVar5 = 0x25c;
      lVar6 = 10;
    }
    fVar18 = (float)(int)*(short *)(p_20 + lVar6 + 0xe) +
             (fVar18 - (float)(int)*(short *)(&UNK_00100d6e + lVar6)) *
             ((float)((int)*(short *)((long)&epa_o3_1h_aqi + lVar6) -
                     (int)*(short *)(p_20 + lVar6 + 0xe)) /
             (float)(iVar5 - *(short *)(&UNK_00100d6e + lVar6)));
  }
LAB_0010020c:
  if (fVar13 <= DAT_00100a04) {
    fVar13 = fVar13 * DAT_00100a40;
  }
  else {
    if (fVar13 <= DAT_00100a08) {
      iVar5 = 0x65;
      lVar6 = 2;
    }
    else if (fVar13 <= DAT_00100a0c) {
      iVar5 = 0x169;
      lVar6 = 4;
    }
    else if (fVar13 <= DAT_00100a10) {
      iVar5 = 0x28a;
      lVar6 = 6;
    }
    else if (fVar13 <= DAT_00100a14) {
      iVar5 = 0x4e2;
      lVar6 = 8;
    }
    else {
      if (DAT_00100a18 < fVar13) {
        fVar13 = DAT_00100998;
        goto LAB_00100258;
      }
      iVar5 = 0x801;
      lVar6 = 10;
    }
    fVar13 = (float)(int)*(short *)(&UNK_00100d7e + lVar6) +
             (fVar13 - (float)(int)*(short *)(&UNK_00100d8e + lVar6)) *
             ((float)((int)*(short *)((long)&epa_no2_1h_aqi + lVar6) -
                     (int)*(short *)(&UNK_00100d7e + lVar6)) /
             (float)(iVar5 - *(short *)(&UNK_00100d8e + lVar6)));
  }
LAB_00100258:
  if (fVar13 < fVar18) {
    fVar13 = fVar18;
  }
  *(short *)(param_9 + 0x1c) = (short)(int)fVar13;
  if (fVar19 <= fVar2) {
    fVar19 = fVar19 * DAT_00100a3c;
  }
  else {
    if (fVar19 <= DAT_001009f0) {
      iVar5 = 0xa5;
      lVar6 = 2;
    }
    else if (fVar19 <= DAT_001009f4) {
      iVar5 = 0xcd;
      lVar6 = 4;
    }
    else if (fVar19 <= DAT_001009f8) {
      iVar5 = 0x195;
      lVar6 = 6;
    }
    else if (fVar19 <= DAT_001009fc) {
      iVar5 = 0x1f9;
      lVar6 = 8;
    }
    else {
      if (DAT_00100a00 < fVar19) {
        fVar19 = DAT_00100998;
        goto LAB_001002b0;
      }
      iVar5 = 0x25c;
      lVar6 = 10;
    }
    fVar19 = (float)(int)*(short *)(p_20 + lVar6 + 0xe) +
             (fVar19 - (float)(int)*(short *)(&UNK_00100d6e + lVar6)) *
             ((float)((int)*(short *)((long)&epa_o3_1h_aqi + lVar6) -
                     (int)*(short *)(p_20 + lVar6 + 0xe)) /
             (float)(iVar5 - *(short *)(&UNK_00100d6e + lVar6)));
  }
LAB_001002b0:
  if (fVar20 <= DAT_00100a1c) {
    fVar20 = fVar20 * DAT_00100a44;
  }
  else {
    if (fVar20 <= DAT_00100a20) {
      iVar5 = 0x47;
      lVar6 = 2;
    }
    else if (fVar20 <= DAT_00100a24) {
      iVar5 = 0x56;
      lVar6 = 4;
    }
    else if (fVar20 <= DAT_00100a28) {
      iVar5 = 0x6a;
      lVar6 = 6;
    }
    else {
      if (DAT_00100a2c < fVar20) {
        fVar20 = DAT_0010099c;
        goto LAB_001002f0;
      }
      iVar5 = 200;
      lVar6 = 8;
    }
    fVar20 = (float)(int)*(short *)(&UNK_00100d9e + lVar6) +
             (fVar20 - (float)(int)*(short *)(&UNK_00100dae + lVar6)) *
             ((float)((int)*(short *)((long)&epa_o3_8h_aqi + lVar6) -
                     (int)*(short *)(&UNK_00100d9e + lVar6)) /
             (float)(iVar5 - *(short *)(&UNK_00100dae + lVar6)));
  }
LAB_001002f0:
  if (fVar8 <= fVar14) {
    fVar8 = fVar8 * DAT_00100a40;
  }
  else {
    if (fVar8 <= DAT_00100a08) {
      iVar5 = 0x65;
      lVar6 = 2;
    }
    else if (fVar8 <= DAT_00100a0c) {
      iVar5 = 0x169;
      lVar6 = 4;
    }
    else if (fVar8 <= DAT_00100a10) {
      iVar5 = 0x28a;
      lVar6 = 6;
    }
    else if (fVar8 <= DAT_00100a14) {
      iVar5 = 0x4e2;
      lVar6 = 8;
    }
    else {
      if (DAT_00100a18 < fVar8) {
        fVar8 = DAT_00100998;
        goto LAB_00100338;
      }
      iVar5 = 0x801;
      lVar6 = 10;
    }
    fVar8 = (float)(int)*(short *)(&UNK_00100d7e + lVar6) +
            (fVar8 - (float)(int)*(short *)(&UNK_00100d8e + lVar6)) *
            ((float)((int)*(short *)((long)&epa_no2_1h_aqi + lVar6) -
                    (int)*(short *)(&UNK_00100d7e + lVar6)) /
            (float)(iVar5 - *(short *)(&UNK_00100d8e + lVar6)));
  }
LAB_00100338:
  if (fVar20 < fVar19) {
    if (fVar8 < fVar19) {
      uVar3 = (undefined2)(int)fVar19;
      goto LAB_00100350;
    }
  }
  else if (fVar8 < fVar20) {
    uVar3 = (undefined2)(int)fVar20;
    goto LAB_00100350;
  }
  uVar3 = (undefined2)(int)fVar8;
LAB_00100350:
  *(undefined2 *)(param_9 + 0x1e) = uVar3;
  uVar4 = 1;
  if (0x32 < (uint)fVar9) {
    if ((param_7[3] < DAT_00100a30) || (DAT_00100a34 < param_7[3])) {
      uVar4 = 0xffffff9a;
    }
    else {
      uVar4 = 0xffffff9a;
      if (param_1 <= DAT_00100a38) {
        uVar4 = 0;
      }
    }
  }
  return uVar4;
}



/* Function: calc_no2_o3 @ 00100a48 */

void calc_no2_o3(undefined8 param_1,long param_2,undefined8 *param_3,float *param_4)

{
  byte bVar1;
  ushort uVar2;
  uint uVar4;
  uint uVar3;
  uint uVar5;
  uint uVar6;
  ushort *puVar7;
  float fVar8;
  
  uVar2 = *(ushort *)(param_2 + 10);
  puVar7 = (ushort *)*param_3;
  uVar5 = (uint)(puVar7[4] >> 8) | (puVar7[4] & 0xff00ff) << 8;
  uVar3 = (uint)uVar2;
  uVar4 = (uint)uVar2;
  uVar6 = (uint)*(ushort *)(param_2 + 8);
  if (uVar2 < uVar5) {
    if (uVar5 < *(ushort *)(param_2 + 8)) {
      fVar8 = ((float)*(byte *)(param_2 + 1) * DAT_00100ce0 * (float)(int)(uVar5 - uVar2)) /
              (float)(int)(uVar6 - uVar5);
      if (fVar8 <= DAT_00100cd8) {
        if (fVar8 < DAT_00100cdc) {
          fVar8 = DAT_00100cdc;
        }
      }
      else {
        fVar8 = DAT_00100cd8;
      }
      goto LAB_00100aec;
    }
    *param_4 = DAT_00100cd8;
    bVar1 = *(byte *)(param_2 + 1);
    uVar5 = (uint)(puVar7[9] >> 8) | (puVar7[9] & 0xff00ff) << 8;
    if (uVar3 < uVar5) goto LAB_00100b08;
LAB_00100a98:
    fVar8 = DAT_00100cdc;
LAB_00100a9c:
    param_4[1] = fVar8;
    bVar1 = *(byte *)(param_2 + 1);
    uVar5 = (uint)(puVar7[0xb] >> 8) | (puVar7[0xb] & 0xff00ff) << 8;
    if (uVar5 <= uVar4) goto LAB_00100b30;
LAB_00100ab8:
    if (uVar6 <= uVar5) {
      param_4[2] = DAT_00100cd8;
      bVar1 = *(byte *)(param_2 + 1);
      uVar5 = (uint)(puVar7[0xf] >> 8) | (puVar7[0xf] & 0xff00ff) << 8;
      goto joined_r0x00100b4c;
    }
    fVar8 = ((float)bVar1 * DAT_00100ce0 * (float)(int)(uVar5 - uVar4)) /
            (float)(int)(uVar6 - uVar5);
    if (fVar8 <= DAT_00100cd8) {
      if (fVar8 < DAT_00100cdc) {
        fVar8 = DAT_00100cdc;
      }
    }
    else {
      fVar8 = DAT_00100cd8;
    }
  }
  else {
    fVar8 = DAT_00100cdc;
LAB_00100aec:
    *param_4 = fVar8;
    bVar1 = *(byte *)(param_2 + 1);
    uVar5 = (uint)(puVar7[9] >> 8) | (puVar7[9] & 0xff00ff) << 8;
    if (uVar5 <= uVar3) goto LAB_00100a98;
LAB_00100b08:
    if (uVar5 < uVar6) {
      fVar8 = ((float)bVar1 * DAT_00100ce0 * (float)(int)(uVar5 - uVar3)) /
              (float)(int)(uVar6 - uVar5);
      if (fVar8 <= DAT_00100cd8) {
        if (fVar8 < DAT_00100cdc) {
          fVar8 = DAT_00100cdc;
        }
      }
      else {
        fVar8 = DAT_00100cd8;
      }
      goto LAB_00100a9c;
    }
    param_4[1] = DAT_00100cd8;
    bVar1 = *(byte *)(param_2 + 1);
    uVar5 = (uint)(puVar7[0xb] >> 8) | (puVar7[0xb] & 0xff00ff) << 8;
    if (uVar3 < uVar5) goto LAB_00100ab8;
LAB_00100b30:
    fVar8 = DAT_00100cdc;
  }
  param_4[2] = fVar8;
  bVar1 = *(byte *)(param_2 + 1);
  uVar5 = (uint)(puVar7[0xf] >> 8) | (puVar7[0xf] & 0xff00ff) << 8;
joined_r0x00100b4c:
  if (uVar4 < uVar5) {
    if (uVar5 < uVar6) {
      fVar8 = ((float)bVar1 * DAT_00100ce0 * (float)(int)(uVar5 - uVar3)) /
              (float)(int)(uVar6 - uVar5);
      if (fVar8 <= DAT_00100cd8) {
        if (fVar8 < DAT_00100cdc) {
          fVar8 = DAT_00100cdc;
        }
      }
      else {
        fVar8 = DAT_00100cd8;
      }
    }
    else {
      fVar8 = DAT_00100cd8;
    }
  }
  else {
    fVar8 = DAT_00100cdc;
  }
  param_4[3] = fVar8;
  calc_no2_o3_internal
            (DAT_00100cec + DAT_00100ce8 * (float)(ushort)(*puVar7 >> 8 | *puVar7 << 8),
             (float)(ushort)(*(ushort *)(param_2 + 3) >> 8 | *(ushort *)(param_2 + 3) << 8) /
             DAT_00100ce4,(float)*(byte *)(param_2 + 6) * 0.5,*(undefined4 *)(param_3 + 1),
             *(undefined4 *)((long)param_3 + 0xc),param_1,param_4,
             *(undefined1 *)(*(long *)(param_2 + 0x10) + 9));
  return;
}



/* Function: log10 @ 00101000 */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

double log10(double __x)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: pow @ 00101008 */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

double pow(double __x,double __y)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: exp @ 00101010 */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

double exp(double __x)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: calc_no2_o3_O3_nnet @ 00101018 */

/* WARNING: Control flow encountered bad instruction data */

void calc_no2_o3_O3_nnet(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: calc_no2_o3_NO2_nnet @ 00101020 */

/* WARNING: Control flow encountered bad instruction data */

void calc_no2_o3_NO2_nnet(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


