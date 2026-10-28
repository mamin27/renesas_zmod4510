/* Program: no2_o3_O3_nnet.c.o */
/* Language: AARCH64:LE:64:v8A */

/* Function: calc_no2_o3_O3_nnet @ 00100000 */

void calc_no2_o3_O3_nnet(undefined8 param_1,undefined8 param_2)

{
  undefined1 auStack_90 [64];
  undefined1 auStack_50 [80];
  
  nnet_funcs_dotbias(param_1,auStack_50,0xe,0x14,&no2_o3_O3_nnet_layer0_bias_weights);
  nnet_funcs_tanh(auStack_50,auStack_50,0x14);
  nnet_funcs_dotbias(auStack_50,auStack_90,0x14,0xf,no2_o3_O3_nnet_layer2_bias_weights);
  nnet_funcs_tanh(auStack_90,auStack_90,0xf);
  nnet_funcs_dotbias(auStack_90,auStack_50,0xf,10,no2_o3_O3_nnet_layer4_bias_weights);
  nnet_funcs_tanh(auStack_50,auStack_50,10);
  nnet_funcs_dotbias(auStack_50,auStack_90,10,5,no2_o3_O3_nnet_layer6_bias_weights);
  nnet_funcs_tanh(auStack_90,auStack_90,5);
  nnet_funcs_dotbias(auStack_90,auStack_50,5,3,no2_o3_O3_nnet_layer8_bias_weights);
  nnet_funcs_tanh(auStack_50,auStack_50,3);
  nnet_funcs_dotbias(auStack_50,param_2,3,1,no2_o3_O3_nnet_layer10_bias_weights);
  return;
}



/* Function: nnet_funcs_dotbias @ 00101000 */

/* WARNING: Control flow encountered bad instruction data */

void nnet_funcs_dotbias(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



/* Function: nnet_funcs_tanh @ 00101008 */

/* WARNING: Control flow encountered bad instruction data */

void nnet_funcs_tanh(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}


