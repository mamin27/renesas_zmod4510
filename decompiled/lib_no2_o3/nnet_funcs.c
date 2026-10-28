/* Program: nnet_funcs.c.o */
/* Language: AARCH64:LE:64:v8A */

/* Function: nnet_funcs_safe_exp @ 00100000 */

float nnet_funcs_safe_exp(float param_1)

{
  double dVar1;
  
  if (DAT_00100050 < param_1) {
    return DAT_00100048;
  }
  if (param_1 < DAT_00100054) {
    return DAT_0010004c;
  }
  dVar1 = exp((double)param_1);
  return (float)dVar1;
}



/* Function: nnet_funcs_tanh @ 00100058 */

void nnet_funcs_tanh(long param_1,long param_2,char param_3)

{
  float fVar1;
  float fVar2;
  long lVar3;
  float fVar4;
  double dVar5;
  undefined4 uVar6;
  
  fVar2 = DAT_00100118;
  fVar1 = DAT_00100114;
  if (param_3 == '\0') {
    return;
  }
  lVar3 = 0;
  do {
    fVar4 = *(float *)(param_1 + lVar3 * 4);
    uVar6 = 0x3f800000;
    if ((fVar1 < fVar4) || (uVar6 = 0xbf800000, fVar4 < fVar2)) {
      *(undefined4 *)(param_2 + lVar3 * 4) = uVar6;
    }
    else {
      dVar5 = exp((double)fVar4);
      fVar4 = (float)dVar5;
      *(float *)(param_2 + lVar3 * 4) = (fVar4 - 1.0 / fVar4) / (fVar4 + 1.0 / fVar4);
    }
    lVar3 = lVar3 + 1;
  } while ((ulong)(byte)(param_3 - 1) + 1 != lVar3);
  return;
}



/* Function: nnet_funcs_relu @ 00100120 */

void nnet_funcs_relu(float *param_1,float *param_2,uint param_3)

{
  uint uVar1;
  float *pfVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  undefined8 *puVar7;
  int iVar8;
  int iVar9;
  bool bVar10;
  ulong uVar11;
  uint uVar12;
  ulong uVar13;
  long lVar14;
  uint uVar15;
  undefined1 uVar16;
  undefined1 uVar17;
  undefined1 uVar18;
  undefined1 uVar19;
  int iVar20;
  int iVar21;
  float fVar22;
  undefined8 uVar23;
  undefined8 uVar24;
  
  uVar4 = param_3 & 0xff;
  if ((param_3 & 0xff) != 0) {
    pfVar2 = param_1 + 4;
    bVar10 = param_2 + 4 > param_1;
    if (((bVar10 && param_2 <= pfVar2) && (param_2 + 4 <= param_1 || pfVar2 != param_2) || uVar4 < 7
        ) || (!bVar10 || (param_2 > pfVar2 || pfVar2 == param_2)) && uVar4 == 7) {
      lVar14 = 0;
      do {
        fVar22 = param_1[lVar14];
        uVar16 = SUB41(fVar22,0);
        uVar17 = (undefined1)((uint)fVar22 >> 8);
        uVar18 = (undefined1)((uint)fVar22 >> 0x10);
        uVar19 = (undefined1)((uint)fVar22 >> 0x18);
        if (fVar22 <= 0.0) {
          uVar16 = 0;
          uVar17 = 0;
          uVar18 = 0;
          uVar19 = 0;
        }
        param_2[lVar14] = (float)CONCAT13(uVar19,CONCAT12(uVar18,CONCAT11(uVar17,uVar16)));
        lVar14 = lVar14 + 1;
      } while (((uint)lVar14 & 0xff) < uVar4);
    }
    else {
      uVar5 = (uint)-((ulong)param_1 >> 2) & 3;
      if ((-((ulong)param_1 >> 2) & 3) == 0) {
        uVar12 = 0;
      }
      else {
        fVar22 = *param_1;
        if (fVar22 <= 0.0) {
          fVar22 = 0.0;
        }
        *param_2 = fVar22;
        uVar12 = uVar5;
        if (uVar5 != 1) {
          fVar22 = param_1[1];
          if (fVar22 <= 0.0) {
            fVar22 = 0.0;
          }
          param_2[1] = fVar22;
          if (uVar5 == 3) {
            fVar22 = param_1[2];
            uVar16 = SUB41(fVar22,0);
            uVar17 = (undefined1)((uint)fVar22 >> 8);
            uVar18 = (undefined1)((uint)fVar22 >> 0x10);
            uVar19 = (undefined1)((uint)fVar22 >> 0x18);
            if (fVar22 <= 0.0) {
              uVar16 = 0;
              uVar17 = 0;
              uVar18 = 0;
              uVar19 = 0;
            }
            param_2[2] = (float)CONCAT13(uVar19,CONCAT12(uVar18,CONCAT11(uVar17,uVar16)));
          }
          else {
            uVar12 = 2;
          }
        }
      }
      uVar3 = uVar4 - uVar5 & 0xff;
      lVar14 = 0;
      uVar15 = 0;
      uVar1 = (uVar3 - 4 >> 2 & 0x3f) + 1;
      uVar6 = uVar1 * 4 & 0xfc;
      do {
        puVar7 = (undefined8 *)((long)param_1 + lVar14 + (ulong)uVar5 * 4);
        uVar24 = puVar7[1];
        uVar23 = *puVar7;
        uVar15 = uVar15 + 1;
        iVar8 = -(uint)(0.0 < (float)uVar23);
        iVar9 = -(uint)(0.0 < (float)((ulong)uVar23 >> 0x20));
        iVar20 = -(uint)(0.0 < (float)uVar24);
        iVar21 = -(uint)(0.0 < (float)((ulong)uVar24 >> 0x20));
        puVar7 = (undefined8 *)((long)param_2 + lVar14 + (ulong)uVar5 * 4);
        puVar7[1] = CONCAT17((byte)((uint)iVar21 >> 0x18) & (byte)((ulong)uVar24 >> 0x38),
                             CONCAT16((byte)((uint)iVar21 >> 0x10) & (byte)((ulong)uVar24 >> 0x30),
                                      CONCAT15((byte)((uint)iVar21 >> 8) &
                                               (byte)((ulong)uVar24 >> 0x28),
                                               CONCAT14((byte)iVar21 & (byte)((ulong)uVar24 >> 0x20)
                                                        ,CONCAT13((byte)((uint)iVar20 >> 0x18) &
                                                                  (byte)((ulong)uVar24 >> 0x18),
                                                                  CONCAT12((byte)((uint)iVar20 >>
                                                                                 0x10) &
                                                                           (byte)((ulong)uVar24 >>
                                                                                 0x10),
                                                                           CONCAT11((byte)((uint)
                                                  iVar20 >> 8) & (byte)((ulong)uVar24 >> 8),
                                                  (byte)iVar20 & (byte)uVar24)))))));
        *puVar7 = CONCAT17((byte)((uint)iVar9 >> 0x18) & (byte)((ulong)uVar23 >> 0x38),
                           CONCAT16((byte)((uint)iVar9 >> 0x10) & (byte)((ulong)uVar23 >> 0x30),
                                    CONCAT15((byte)((uint)iVar9 >> 8) &
                                             (byte)((ulong)uVar23 >> 0x28),
                                             CONCAT14((byte)iVar9 & (byte)((ulong)uVar23 >> 0x20),
                                                      CONCAT13((byte)((uint)iVar8 >> 0x18) &
                                                               (byte)((ulong)uVar23 >> 0x18),
                                                               CONCAT12((byte)((uint)iVar8 >> 0x10)
                                                                        & (byte)((ulong)uVar23 >>
                                                                                0x10),
                                                                        CONCAT11((byte)((uint)iVar8
                                                                                       >> 8) &
                                                                                 (byte)((ulong)
                                                  uVar23 >> 8),(byte)iVar8 & (byte)uVar23)))))));
        lVar14 = lVar14 + 0x10;
      } while ((uVar15 & 0xff) < uVar1);
      uVar12 = uVar12 + uVar6;
      if (uVar3 != uVar6) {
        uVar13 = (ulong)(uVar12 + 1) & 0xff;
        fVar22 = param_1[uVar12];
        uVar16 = SUB41(fVar22,0);
        uVar17 = (undefined1)((uint)fVar22 >> 8);
        uVar18 = (undefined1)((uint)fVar22 >> 0x10);
        uVar19 = (undefined1)((uint)fVar22 >> 0x18);
        if (fVar22 <= 0.0) {
          uVar16 = 0;
          uVar17 = 0;
          uVar18 = 0;
          uVar19 = 0;
        }
        param_2[uVar12] = (float)CONCAT13(uVar19,CONCAT12(uVar18,CONCAT11(uVar17,uVar16)));
        if ((uint)uVar13 < uVar4) {
          fVar22 = param_1[uVar13];
          uVar16 = SUB41(fVar22,0);
          uVar17 = (undefined1)((uint)fVar22 >> 8);
          uVar18 = (undefined1)((uint)fVar22 >> 0x10);
          uVar19 = (undefined1)((uint)fVar22 >> 0x18);
          uVar11 = (ulong)(uVar12 + 2) & 0xff;
          if (fVar22 <= 0.0) {
            uVar16 = 0;
            uVar17 = 0;
            uVar18 = 0;
            uVar19 = 0;
          }
          param_2[uVar13] = (float)CONCAT13(uVar19,CONCAT12(uVar18,CONCAT11(uVar17,uVar16)));
          if (uVar4 <= (uint)uVar11) {
            return;
          }
          fVar22 = param_1[uVar11];
          uVar16 = SUB41(fVar22,0);
          uVar17 = (undefined1)((uint)fVar22 >> 8);
          uVar18 = (undefined1)((uint)fVar22 >> 0x10);
          uVar19 = (undefined1)((uint)fVar22 >> 0x18);
          if (fVar22 <= 0.0) {
            uVar16 = 0;
            uVar17 = 0;
            uVar18 = 0;
            uVar19 = 0;
          }
          param_2[uVar11] = (float)CONCAT13(uVar19,CONCAT12(uVar18,CONCAT11(uVar17,uVar16)));
          return;
        }
      }
    }
  }
  return;
}



/* Function: nnet_funcs_dotbias @ 00100298 */

void nnet_funcs_dotbias(long param_1,long param_2,uint param_3,uint param_4,float *param_5)

{
  long lVar1;
  float *pfVar2;
  long lVar3;
  float *pfVar4;
  long lVar5;
  float fVar6;
  
  param_3 = param_3 & 0xff;
  if ((param_4 & 0xff) != 0) {
    lVar5 = 0;
    do {
      pfVar2 = param_5 + 1;
      fVar6 = *param_5;
      *(float *)(param_2 + lVar5 * 4) = fVar6;
      if (param_3 != 0) {
        lVar3 = 0;
        pfVar4 = pfVar2;
        do {
          lVar1 = lVar3 * 4;
          lVar3 = lVar3 + 1;
          fVar6 = fVar6 + *(float *)(param_1 + lVar1) * *pfVar4;
          *(float *)(param_2 + lVar5 * 4) = fVar6;
          pfVar4 = pfVar4 + 1;
        } while (((uint)lVar3 & 0xff) < param_3);
        pfVar2 = pfVar2 + ((ulong)(param_3 - 1) & 0xff) + 1;
      }
      lVar5 = lVar5 + 1;
      param_5 = pfVar2;
    } while (((uint)lVar5 & 0xff) < (param_4 & 0xff));
  }
  return;
}



/* Function: exp @ 00101000 */

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

double exp(double __x)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


