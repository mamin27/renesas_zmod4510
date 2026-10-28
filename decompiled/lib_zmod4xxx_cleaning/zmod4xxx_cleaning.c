/* Program: zmod4xxx_cleaning.c.o */
/* Language: AARCH64:LE:64:v8A */

/* Function: zmod4xxx_cleaning_run @ 00100000 */

int zmod4xxx_cleaning_run(ulong *param_1)

{
  uint uVar1;
  char cVar2;
  uint uVar3;
  ushort uVar4;
  byte local_42;
  char local_41;
  ulong local_40;
  ulong uStack_38;
  ulong local_30;
  ulong uStack_28;
  code *local_20;
  code *local_18;
  ulong local_10;
  undefined1 *local_8;
  
  local_40 = *param_1;
  uStack_38 = param_1[1];
  local_30 = param_1[2];
  uStack_28 = param_1[3];
  local_10 = param_1[6];
  local_20 = (code *)param_1[4];
  local_18 = (code *)param_1[5];
  local_42 = 0;
  local_8 = &zmod4xxx_cleaning;
  cVar2 = (*(code *)param_1[3])((char)*param_1,0x38,&local_41,1);
  if (cVar2 == '\0') {
    if (local_41 != '\0') {
      return -8;
    }
    cVar2 = zmod4xxx_init_measurement(&local_40);
    if (cVar2 != 0) {
      return (int)cVar2;
    }
    uVar4 = 0;
    cVar2 = zmod4xxx_start_measurement(&local_40);
    uVar3 = 0;
    if (cVar2 != 0) {
      return (int)cVar2;
    }
    while( true ) {
      (*local_18)(0x32);
      cVar2 = zmod4xxx_read_status(&local_40,&local_42);
      if (cVar2 != 0) {
        return (int)cVar2;
      }
      uVar1 = local_42 & 0x1f;
      if (((uVar3 != uVar1) && (uVar1 == ((byte)local_8[0x39] >> 1) - 1)) &&
         (uVar4 = uVar4 + 1, 0x1d < uVar4)) break;
      uVar3 = uVar1;
    }
    cVar2 = (*local_20)(local_40 & 0xff,0xffffff93,&stop_cleaning_code,1);
    if (cVar2 == '\0') {
      do {
        (*local_18)(0x32);
        cVar2 = zmod4xxx_read_status(&local_40,&local_42);
        if (cVar2 != 0) {
          return (int)cVar2;
        }
      } while ((char)local_42 < '\0');
      (*local_18)(0x32);
      cVar2 = (*local_20)(local_40 & 0xff,0xffffffb8,&enable_write_nvm_code,1);
      if (cVar2 == '\0') {
        (*local_18)(0x32);
        local_41 = local_41 + '\x01';
        cVar2 = (*local_20)(local_40 & 0xff,0x38,&local_41,1);
        if (cVar2 == '\0') {
          (*local_18)(0x32);
          cVar2 = (*local_20)(local_40 & 0xff,0xffffffb8,&disable_write_nvm_code,1);
          return (int)cVar2;
        }
      }
    }
  }
  return -3;
}



/* Function: zmod4xxx_init_measurement @ 00101000 */

/* WARNING: Control flow encountered bad instruction data */

void zmod4xxx_init_measurement(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: zmod4xxx_start_measurement @ 00101008 */

/* WARNING: Control flow encountered bad instruction data */

void zmod4xxx_start_measurement(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: zmod4xxx_read_status @ 00101010 */

/* WARNING: Control flow encountered bad instruction data */

void zmod4xxx_read_status(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


