// logica do interpretador com implementaçoes de instruçoes

// 7.    Opcode Mnemonics by Opcode -> https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-7.html
// 2.11. Instruction Set Summary    -> https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-2.html#jvms-2.11
// 6.5.  Instructions (detalhes)    -> https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5
// https://en.wikipedia.org/wiki/Java_bytecode_instruction_listings

// interpreter.h
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "checker.h"

// TIPO DE PONTEIRO PRA FUNÇÃO QUE VAI EXECUTAR A RESPECTIVA INSTRUÇÃO DA JVM
// Define um ponteiro pra função chamado INSTRUCTION que retorna void e recebe METHOD_DATA*, 
// THREAD* e JVM* como parâmetros.
typedef void (*INSTRUCTION)(METHOD_DATA *, THREAD *, JVM *);

// ARRAY DE PONTEIROS PRA FUNÇÕES
// func eh um array em que cada posiçao guarda um ponteiro pra funcao do tipo INSTRUCTION
extern INSTRUCTION func[];


//-- interpretador

// INTERPRETADOR DE BYTECODES
// executa todas as instrucoes
void interpreter(METHOD_DATA*, THREAD*, JVM*);


//-- instrucoes

// CONTANTES - Constants
// --  Carregamento de constantes na pilha de operandos.
	// 00  (0x00)    nop 				> Do nothing
	// --  Faz nada.
	void nop_(METHOD_DATA *, THREAD *, JVM *);
	// 01  (0x01)    aconst_null 		> Push null
	// 02  (0x02)    iconst_m1 			> Push int constant (-1)
	// 03  (0x03)    iconst_0			> Push int constant (0)
	// 04  (0x04)    iconst_1			> Push int constant (1)
	// 05  (0x05)    iconst_2			> Push int constant (2)
	// 06  (0x06)    iconst_3			> Push int constant (3)
	// 07  (0x07)    iconst_4			> Push int constant (4)
	// 08  (0x08)    iconst_5			> Push int constant (5)
	// 09  (0x09)    lconst_0			> Push long constant (0)
	// 10  (0x0a)    lconst_1			> Push long constant (1)
	// 11  (0x0b)    fconst_0			> Push float constant (0.0)
	// 12  (0x0c)    fconst_1			> Push float constant (1.0)
	// 13  (0x0d)    fconst_2			> Push float constant (2.0)
	// 14  (0x0e)    dconst_0			> Push double constant (0.0)
	// 15  (0x0f)    dconst_1			> Push double constant (1.0)
	// --  Carregam constantes na pilha de operandos.
	void Tconst(METHOD_DATA *, THREAD *, JVM *);
	// 16  (0x10)    bipush 			> Push byte (byte imediato como int com sinal)
	// 17  (0x11)    sipush 			> Push short (byte1 e byte2 imediatos concatenados sem sinal como um short com sinal)
	// --  Carregam parametros inteiros com sinal (16-short ou 8-byte) na pilha 
	// --  de operandos.
	void Tipush(METHOD_DATA *, THREAD *, JVM *);
	// 18  (0x12)    ldc 				> Push item from run-time constant pool (indice eh byte sem sinal valido da cp atual da classe atual)
	// 19  (0x13)    ldc_w 				> Push item from run-time constant pool (wide index - usa  indexbyte1 e  indexbyte2 concatenado)
	// 20  (0x14)    ldc2_w 			> Push long or double from run-time constant pool (wide index - usa  indexbyte1 e  indexbyte2 concatenado) 
	// --  Carregam constantes single (int, float, literal string) ou
	// --  double word (long, double) na pilha de operandos.
	// --  Tem como argumento um indice de 8 ou 16 bits da constant pool
	void ldc_(METHOD_DATA *, THREAD *, JVM *);
// CARREGAMENTO - Loads
// --  Carregam na pilha de operandos valores do vetor de variaveis locais
// --  ou de arrays referenciados pelo vetor.
// --  Tem como argumento indices para o vetor ou indices implicitos.
	// 21  (0x15)    iload 			> Load int from local variable (indice eh byte sem sinal do array de variaveis do frame atual)
	// 22  (0x16)    lload 			> Load long from local variable (tanto indice quanto indice+1 valido no array do frame atual)
	// 23  (0x17)    fload 			> Load float from local variable (indice eh byte sem sinal do array de variaveis do frame atual)
	// 24  (0x18)    dload 			> Load double from local variable (tanto indice quanto indice+1 valido no array do frame atual)
	// 25  (0x19)    aload 			> Load reference from local variable (indice eh byte sem sinal do array de variaveis do frame atual) - objectref
	// 26  (0x1a)    iload_0		> Load int from local variable (0 eh o indice do vetor do frame atual)
	// 27  (0x1b)    iload_1		> Load int from local variable (1 eh o indice do vetor do frame atual)
	// 28  (0x1c)    iload_2		> Load int from local variable (2 eh o indice do vetor do frame atual)
	// 29  (0x1d)    iload_3		> Load int from local variable (3 eh o indice do vetor do frame atual)
	// 30  (0x1e)    lload_0		> Load long from local variable (tanto 0 quanto 1 indices validos no array do frame atual)
	// 31  (0x1f)    lload_1		> Load long from local variable (tanto 1 quanto 2 indices validos no array do frame atual)
	// 32  (0x20)    lload_2		> Load long from local variable (tanto 2 quanto 3 indices validos no array do frame atual)
	// 33  (0x21)    lload_3		> Load long from local variable (tanto 3 quanto 4 indices validos no array do frame atual)
	// 34  (0x22)    fload_0		> Load float from local variable (0 eh o indice do vetor do frame atual)
	// 35  (0x23)    fload_1		> Load float from local variable (1 eh o indice do vetor do frame atual)
	// 36  (0x24)    fload_2		> Load float from local variable (2 eh o indice do vetor do frame atual)
	// 37  (0x25)    fload_3		> Load float from local variable (3 eh o indice do vetor do frame atual)
	// 38  (0x26)    dload_0		> Load double from local variable (tanto 0 quanto 1 indices validos no array do frame atual) 
	// 39  (0x27)    dload_1		> Load double from local variable (tanto 1 quanto 2 indices validos no array do frame atual) 
	// 40  (0x28)    dload_2		> Load double from local variable (tanto 2 quanto 3 indices validos no array do frame atual) 
	// 41  (0x29)    dload_3		> Load double from local variable (tanto 3 quanto 4 indices validos no array do frame atual) 
	// 42  (0x2a)    aload_0 		> Load reference from local variable (0 eh o indice do vetor do frame atual)
	// 43  (0x2b)    aload_1 		> Load reference from local variable (1 eh o indice do vetor do frame atual)
	// 44  (0x2c)    aload_2 		> Load reference from local variable (2 eh o indice do vetor do frame atual)
	// 45  (0x2d)    aload_3		> Load reference from local variable (3 eh o indice do vetor do frame atual)
	// --  Carregam na pilha de operandos valores do vetor 
	// --  de variaveis locais.
	void Tload(METHOD_DATA *, THREAD *, JVM *);
	// 46  (0x2e)    iaload 		> Load int from array (arrayref de tipo reference e refere a array de inteiros)
	// 47  (0x2f)    laload 		> Load long from array (arrayref de tipo reference e refere a array de longs)
	// 48  (0x30)    faload			> Load float from array  (arrayref de tipo reference e refere a array de floats)
	// 49  (0x31)    daload 		> Load double from array (arrayref de tipo reference e refere a array de floats)
	// 50  (0x32)    aaload 		> Load reference from array (arrayref de tipo reference e refere a array de references)
	// 51  (0x33)    baload 		> Load byte or boolean from array (arrayref de tipo reference e refere a array de byte ou boolean)
	// 52  (0x34)    caload 		> Load char from array (arrayref de tipo reference e refere a array de chars)
	// 53  (0x35)    saload 		> Load short from array (arrayref de tipo reference e refere a array de shorts)
	// --  Carregam na pilha de operandos valores de arrays 
	// --  referenciados pela pilha.
	void Taload(METHOD_DATA *, THREAD *, JVM *);
// ARMAZENAMENTO - Stores
// --  Grava valores do topo da pilha de operandos no vetor de variaveis locais 
// --  ou em arrays referenciados pela pilha.
	// 54  (0x36)    istore 		> Store int into local variable (indice eh byte sem sinal valido no array do frame atual)
	// 55  (0x37)    lstore 		> Store long into local variable (tanto indice quanto indice+1 valido no array do frame atual)
	// 56  (0x38)    fstore 		> Store float into local variable (indice eh byte sem sinal valido no array do frame atual)
	// 57  (0x39)    dstore 		> Store double into local variable (tanto indice quanto indice+1 valido no array do frame atual)
	// 58  (0x3a)    astore 		> Store reference into local variable (indice eh byte sem sinal valido no array do frame atual) - returnAddress ou reference
	// 59  (0x3b)    istore_0		> Store int into local variable (0 eh o indice do vetor do frame atual)
	// 60  (0x3c)    istore_1		> Store int into local variable (1 eh o indice do vetor do frame atual)
	// 61  (0x3d)    istore_2		> Store int into local variable (2 eh o indice do vetor do frame atual)
	// 62  (0x3e)    istore_3		> Store int into local variable (3 eh o indice do vetor do frame atual)
	// 63  (0x3f)    lstore_0		> Store long into local variable (tanto 0 quanto 1 indices validos no array do frame atual)
	// 64  (0x40)    lstore_1		> Store long into local variable (tanto 1 quanto 2 indices validos no array do frame atual)
	// 65  (0x41)    lstore_2		> Store long into local variable (tanto 2 quanto 3 indices validos no array do frame atual)
	// 66  (0x42)    lstore_3		> Store long into local variable (tanto 3 quanto 4 indices validos no array do frame atual)
	// 67  (0x43)    fstore_0		> Store float into local variable (0 eh o indice do vetor do frame atual)
	// 68  (0x44)    fstore_1		> Store float into local variable (1 eh o indice do vetor do frame atual)
	// 69  (0x45)    fstore_2		> Store float into local variable (2 eh o indice do vetor do frame atual)
	// 70  (0x46)    fstore_3		> Store float into local variable (3 eh o indice do vetor do frame atual)
	// 71  (0x47)    dstore_0		> Store double into local variable (tanto 0 quanto 1 indices validos no array do frame atual)
	// 72  (0x48)    dstore_1		> Store double into local variable (tanto 1 quanto 2 indices validos no array do frame atual)
	// 73  (0x49)    dstore_2		> Store double into local variable (tanto 2 quanto 3 indices validos no array do frame atual)
	// 74  (0x4a)    dstore_3		> Store double into local variable (tanto 3 quanto 4 indices validos no array do frame atual)
	// 75  (0x4b)    astore_0		> Store reference into local variable (0 eh o indice do vetor do frame atual)
	// 76  (0x4c)    astore_1		> Store reference into local variable (1 eh o indice do vetor do frame atual)
	// 77  (0x4d)    astore_2		> Store reference into local variable (2 eh o indice do vetor do frame atual)
	// 78  (0x4e)    astore_3		> Store reference into local variable (3 eh o indice do vetor do frame atual)
	// --  Grava valores do topo da pilha de operandos no vetor de variaveis locais.
	void Tstore(METHOD_DATA *, THREAD *, JVM *);
	// 79  (0x4f)    iastore 		> Store into int array (arrayref de tipo reference e refere a array de inteiros) 
	// 80  (0x50)    lastore 		> Store into long array (arrayref de tipo reference e refere a array de longs) 
	// 81  (0x51)    fastore 		> Store into float array (arrayref de tipo reference e refere a array de floats) 
	// 82  (0x52)    dastore 		> Store into double array (arrayref de tipo reference e refere a array de doubles) 
	// 83  (0x53)    aastore 		> Store into reference array (arrayref de tipo reference e refere a array de references) 
	// 84  (0x54)    bastore 		> Store into byte or boolean array (arrayref de tipo reference e refere a array de byte ou boolean) 
	// 85  (0x55)    castore 		> Store into char array (arrayref de tipo reference e refere a array de chars) 
	// 86  (0x56)    sastore 		> Store into short array (arrayref de tipo reference e refere a array de shorts) 
	// --  Grava valores do topo da pilha de operandos em arrays referenciados pela 
	// --  pilha.
	void Tastore(METHOD_DATA *, THREAD *, JVM *);
// PILHA - Stack
	// 87  (0x57)    pop 		> Pop the top operand stack value (da pilha de operandos)
	// 88  (0x58)    pop2 		> Pop the top one or two operand stack values (dois valores cat1 ou um valor cat2 da pilha de operandos)
	// 89  (0x59)    dup 		> Duplicate the top operand stack value (duplica valor do topo da pilha na pilha)
	// 90  (0x5a)    dup_x1 	> Duplicate the top operand stack value and insert two values down 
	// 91  (0x5b)    dup_x2 	> Duplicate the top operand stack value and insert two or three values down (todos cat1 ou um cat1 e um cat2)
	// 92  (0x5c)    dup2 		> Duplicate the top one or two operand stack values (dois valores cat1 ou um valor cat2 da pilha de operandos)
	// 93  (0x5d)    dup2_x1 	> Duplicate the top one or two operand stack values and insert two or three values down (todos cat1 ou um cat1 e um cat2)
	// 94  (0x5e)    dup2_x2 	> Duplicate the top one or two operand stack values and insert two, three, or four values down
	// 95  (0x5f)    swap 		> Swap the top two operand stack values (ambos devem ser cat1)
	// --  Para manipulacao da pilha de operandos.
	// --  Descarte, duplicacao, troca de valores.
	void handleStack(METHOD_DATA *, THREAD *, JVM *);
// MATEMATICAS - Math
// --  Instrucoes logico-aritmeticas que manipulam operandos na pilha de operandos.
	// 96  (0x60)    iadd 		> Add int (pop 2 inteiros e push resultado)
	// 97  (0x61)    ladd 		> Add long (pop 2 long e push resultado)
	// 98  (0x62)    fadd 		> Add float (pop 2 float e push resultado)
	// 99  (0x63)    dadd 		> Add double (pop 2 double e push resultado)
	// --  De adicao.
	void Tadd(METHOD_DATA *, THREAD *, JVM *);
	// 100 (0x64)    isub 		> Subtract int (pop 2 inteiros e push resultado)
	// 101 (0x65)    lsub 		> Subtract long (pop 2 long e push resultado)
	// 102 (0x66)    fsub 		> Subtract float (pop 2 float e push resultado)
	// 103 (0x67)    dsub 		> Subtract double (pop 2 double e push resultado)
	// --  De subtracao.
	void Tsub(METHOD_DATA *, THREAD *, JVM *);
	// 104 (0x68)    imul		> Multiply int (pop 2 inteiros e push resultado)
	// 105 (0x69)    lmul		> Multiply long (pop 2 long e push resultado)
	// 106 (0x6a)    fmul		> Multiply float (pop 2 float e push resultado)
	// 107 (0x6b)    dmul		> Multiply double (pop 2 double e push resultado)
	// --  De multiplicacao.
	void Tmul(METHOD_DATA *, THREAD *, JVM *);
	// 108 (0x6c)    idiv		> Divide int (pop 2 inteiros e push resultado) 
	// 109 (0x6d)    ldiv		> Divide long (pop 2 long e push resultado)
	// 110 (0x6e)    fdiv		> Divide float (pop 2 float e push resultado)
	// 111 (0x6f)    ddiv		> Divide double (pop 2 double e push resultado)
	// --  De divisao.
	void Tdiv(METHOD_DATA *, THREAD *, JVM *);
	// 112 (0x70)    irem		> Remainder int (pop 2 inteiros e push resultado) 
	// 113 (0x71)    lrem		> Remainder long (pop 2 long e push resultado) 
	// 114 (0x72)    frem		> Remainder float (pop 2 float e push resultado) 
	// 115 (0x73)    drem		> Remainder double (pop 2 double e push resultado) 
	// -- De resto.
	void Trem(METHOD_DATA *, THREAD *, JVM *);
	// 116 (0x74)    ineg		> Negate int (pop 1 inteiro e push resultado) 
	// 117 (0x75)    lneg		> Negate long (pop 1 long e push resultado) 
	// 118 (0x76)    fneg		> Negate float (pop 1 float e push resultado) 
	// 119 (0x77)    dneg		> Negate double (pop 1 double e push resultado) 
	// --  De negacao.
	void Tneg(METHOD_DATA *, THREAD *, JVM *);
	// 120 (0x78)    ishl 		> Shift left int (pop 2 inteiros e push resultado) - 5 bits e extende sinal
	// 121 (0x79)    lshl 		> Shift left long (pop 1 long e 1 int e push resultado) - 6 bits e extende sinal
	// --  De deslocamento a esquerda.
	void Tshl(METHOD_DATA *, THREAD *, JVM *);
	// 122 (0x7a)    ishr 		> Arithmetic shift right int (pop 2 inteiros e push resultado) - 5 bits e extende sinal
	// 123 (0x7b)    lshr 		> Arithmetic shift right long (pop 1 long e 1 int e push resultado) - 6 bits e extende sinal
	// --  De deslocamento a direita.
	void Tshr(METHOD_DATA *, THREAD *, JVM *);
	// 124 (0x7c)    iushr 		> Logical shift right int (pop 2 inteiros e push resultado) - 5 bits e extende zeros
	// 125 (0x7d)    lushr 		> Logical shift right long (pop 1 long e 1 int e push resultado) - 6 bits e extende zeros
	// --  De deslocamento a direita sem sinal.
	void Tushr(METHOD_DATA *, THREAD *, JVM *);
	// 126 (0x7e)    iand 		> Boolean AND int (pop 2 inteiros e push resultado) 
	// 127 (0x7f)    land 		> Boolean AND long (pop 2 long e push resultado) 
	// --  De AND bit a bit.
	void Tand(METHOD_DATA *, THREAD *, JVM *);
	// 128 (0x80)    ior 		> Boolean OR int (pop 2 inteiros e push resultado) 
	// 129 (0x81)    lor	 	> Boolean OR long (pop 2 long e push resultado) 
	// --  De OR bit a bit.
	void Tor(METHOD_DATA *, THREAD *, JVM *);
	// 130 (0x82)    ixor 		> Boolean XOR int (pop 2 inteiros e push resultado) 
	// 131 (0x83)    lxor 		> Boolean XOR long (pop 2 long e push resultado) 
	// --  De XOR bit a bit.
	void Txor(METHOD_DATA *, THREAD *, JVM *);
	// 132 (0x84)    iinc 		> Increment local variable by constant (incrementa variavel local em indice valido de constante imediata com sinal)
	// --  De incremento de constante a posicao no array de variaveis locais.
	void Tinc(METHOD_DATA *, THREAD *, JVM *);
// CONVERSOES - Conversions
// --  Instrucoes que convertem valores presentes na pilha de operandos 
// --  entre diferentes tipos.
	// 133 (0x85)    i2l 		> Convert int to long (pop 1 inteiro e push resultado) 
	// 134 (0x86)    i2f 		> Convert int to float (pop 1 inteiro e push resultado)
	// 135 (0x87)    i2d 		> Convert int to double (pop 1 inteiro e push resultado)
	// 145 (0x91)    i2b 		> Convert int to byte (pop 1 inteiro e push resultado)
	// 146 (0x92)    i2c 		> Convert int to char (pop 1 inteiro e push resultado)
	// 147 (0x93)    i2s 		> Convert int to short (pop 1 inteiro e push resultado)
	// --  Convertem de inteiro pra um outro tipo.
	void i2T(METHOD_DATA *, THREAD *, JVM *);
	// 136 (0x88)    l2i 		> Convert long to int (pop 1 long e push resultado) 
	// 137 (0x89)    l2f 		> Convert long to float (pop 1 long e push resultado) 
	// 138 (0x8a)    l2d 		> Convert long to double (pop 1 long e push resultado) 
	// --  Convertem de long para um outro tipo.
	void l2T(METHOD_DATA *, THREAD *, JVM *);
	// 139 (0x8b)    f2i 		> Convert float to int (pop 1 float e push resultado) 
	// 140 (0x8c)    f2l 		> Convert float to long (pop 1 float e push resultado) 
	// 141 (0x8d)    f2d 		> Convert float to double (pop 1 float e push resultado) 
	// --  Convertem de float para um outro tipo.
	void f2T(METHOD_DATA *, THREAD *, JVM *);
	// 142 (0x8e)    d2i 		> Convert double to int (pop 1 double e push resultado) 
	// 143 (0x8f)    d2l 		> Convert double to long (pop 1 double e push resultado) 
	// 144 (0x90)    d2f 		> Convert double to float (pop 1 double e push resultado) 
	// --  Convertem de double para um outro tipo.
	void d2T(METHOD_DATA *, THREAD *, JVM *);
// COMPARACOES - Comparisons
	// 148 (0x94)    lcmp 		> Compare long (pop 2 long e push 1, 0 ou value-1)
	// -- Comparacao de tipo integral (long).
	void Tcmp(METHOD_DATA *, THREAD *, JVM *);
	// 149 (0x95)    fcmpl 		> Compare float (pop 2 float e push 1, 0 ou -1) - se algum NaN push -1
	// 150 (0x96)    fcmpg 		> Compare float (pop 2 float e push 1, 0 ou -1) - se algum NaN push 1
	// 151 (0x97)    dcmpl 		> Compare double (pop 2 double e push 1, 0 ou -1) - se algum NaN push -1
	// 152 (0x98)    dcmpg 		> Compare double (pop 2 double e push 1, 0 ou -1) - se algum NaN push 1
	// --  Comparacao de ponto flutuante.
	void TcmpOP(METHOD_DATA *, THREAD *, JVM *);
	// 153 (0x99)    ifeq		> Branch if int comparison with zero succeeds (pop 1 int e compara com zero) =0
	// 154 (0x9a)    ifne		> Branch if int comparison with zero succeeds (pop 1 int e compara com zero) !=0
	// 155 (0x9b)    iflt		> Branch if int comparison with zero succeeds (pop 1 int e compara com zero) <0
	// 156 (0x9c)    ifge		> Branch if int comparison with zero succeeds (pop 1 int e compara com zero) <=0
	// 157 (0x9d)    ifgt		> Branch if int comparison with zero succeeds (pop 1 int e compara com zero) >0
	// 158 (0x9e)    ifle		> Branch if int comparison with zero succeeds (pop 1 int e compara com zero) >=0
	// --  Compara valor inteiro com zero e realiza um jump.
	void ifOP(METHOD_DATA *, THREAD *, JVM *);
	// 159 (0x9f)    if_icmpeq	> Branch if int comparison succeeds (pop 2 int e compara entre eles) value1 = value2
	// 160 (0xa0)    if_icmpne	> Branch if int comparison succeeds (pop 2 int e compara entre eles) value1 != value2
	// 161 (0xa1)    if_icmplt	> Branch if int comparison succeeds (pop 2 int e compara entre eles) value1 < value2
	// 162 (0xa2)    if_icmpge	> Branch if int comparison succeeds (pop 2 int e compara entre eles) value1 <= value2
	// 163 (0xa3)    if_icmpgt	> Branch if int comparison succeeds (pop 2 int e compara entre eles) value1 > value2
	// 164 (0xa4)    if_icmple	> Branch if int comparison succeeds (pop 2 int e compara entre eles) value1 >= value2
	// --  Compara dois valores inteiros e realiza um jump.
	void if_icmOP(METHOD_DATA *, THREAD *, JVM *);
	// 165 (0xa5)    if_acmpeq	> Branch if reference comparison succeeds (pop 2 reference e compara entre eles) value1 = value2
	// 166 (0xa6)    if_acmpne	> Branch if reference comparison succeeds (pop 2 reference e compara entre eles) value1 != value2	
	// --  Compara dois valores do tipo reference.
	void if_acmOP(METHOD_DATA *, THREAD *, JVM *);
// CONTROLE - Control
	// 167 (0xa7)    goto 		> Branch always (dois bytes sem sinal concatenados de offset)
	// 168 (0xa8)    jsr 		> Jump subroutine (push endereço de opcode seguinte na pilha como um returnAddress, offset de dois bytes concatenados)
	// 169 (0xa9)    ret 		> Return from subroutine (returnAddress no array no indice escrito no pc)
	// --  Realiza jumps incondicionais.
	void jump(METHOD_DATA *, THREAD *, JVM *);
	// 170 (0xaa)    tableswitch 	> Access jump table by index and jump (uma tableswitch eh uma instrucao de tamanho variavel)
	// 171 (0xab)    lookupswitch 	> Access jump table by key match and jump (uma lookupswitch eh uma instrucao de tamanho variavel)
	// --  Realiza jumps compostos.
	void switch_(METHOD_DATA *, THREAD *, JVM *);
	// 172 (0xac)    ireturn 		> Return int from method (metodo corrente deve retornar boolean, byte, short, char, ou int, valor inteiro, controle ao invocador do metodo)
	// 173 (0xad)    lreturn 		> Return long from method (metodo corrente deve retornar long, valor long, controle ao invocador do metodo)
	// 174 (0xae)    freturn 		> Return float from method (metodo corrente deve retornar float, valor float, controle ao invocador do metodo)
	// 175 (0xaf)    dreturn		> Return double from method (metodo corrente deve retornar long, valor long, controle ao invocador do metodo)
	// 176 (0xb0)    areturn 		> Return reference from method (objectref eh um reference referente a objeto de mesmo tipo do descritor de retorno do metodo)
	// 177 (0xb1)    return 		> Return void from method (metodo corrente deve retornar void, controle ao invocador do metodo)
	// --  Retorna valor do metodo e retoma a execucao do metodo invocador.
	void Treturn(METHOD_DATA *, THREAD *, JVM *);
// REFERENCIAS - References
	// 178 (0xb2)    getstatic 		> Get static field from class (dois bytes indice da cp da classe eh ref de name e descritor de field e classe ou interface, que eh iniciada e seu campo colocado na pilha)
	// 179 (0xb3)    putstatic 		> Set static field in class (dois bytes indice da cp da classe eh ref de name e descritor de field e classe ou interface, tipo colocado na pilha depende)
	// 180 (0xb4)    getfield 		> Fetch field from object ( reference objectref eh tirado da pilha, dois bytes indice da cp da classe eh ref de name e descritor de field e classe ou interface, valor objectref colocado na pilha)
	// 181 (0xb5)    putfield 		> Set field in object (dois bytes indice da cp da classe eh ref de name e descritor de field e classe ou interface, tipo colocado na pilha depende)
	// --  Manipula fields.
	void accessField(METHOD_DATA *, THREAD *, JVM *);
	// 182 (0xb6)    invokevirtual 	> Invoke instance method; dispatch based on class
	// 183 (0xb7)    invokespecial 	> Invoke instance method; special handling for superclass, private, and instance initialization method invocations
	// 184 (0xb8)    invokestatic 	> Invoke a class (static) method 
	// 185 (0xb9)    invokeinterface> Invoke interface method
	// 186 (0xba)    invokedynamic	> Invoke dynamic method 
	// --  Invocacao de metodos.
	void invoke(METHOD_DATA *, THREAD *, JVM *);
	// 187 (0xbb)    new 			> Create new object (dois bytes sao indice da cp que indica ref simbolica a classe ou interface, mem pra instancia eh alocada na pilha, var de instancia com valores padrao, referencia a instancia colocada na pilha)
	// 188 (0xbc)    newarray 		> Create new array (contador tirado da pilha como num de elementos do array, atype eh indice para tipo a ser criado, array alocado na heap)
	// 189 (0xbd)    anewarray 		> Create new array of reference (contador tirado da pilha como num de elementos do array de tipo reference, array alocado na heap)
	// 190 (0xbe)    arraylength 	> Get length of array (ref ao array tirada da pilha, obtem tamanho do array e coloca na pilha como int)
	// --  Criacao e manipulacao de objetos e arrays.
	void handleObject(METHOD_DATA *, THREAD *, JVM *);
	// 191 (0xbf)    athrow 		> Throw exception or error (ref de objeto Throwable ou subclasse, tirado da pilha, pc resetado, limpa frame, continua executar)
	// --  Lancamento de excessoes
	void athrow(METHOD_DATA *, THREAD *, JVM *);
	// 192 (0xc0)    checkcast 		> Check whether object is of given type (dois bytes indice de cp)
	// 193 (0xc1)    instanceof 	> Determine if object is of given type (tira ref da pilha, dois bytes indice da cp)
	// --  Checa propriedades de objetos.
	void properties(METHOD_DATA *, THREAD *, JVM *);
	// 194 (0xc2)    monitorenter 	> Enter monitor for object
	// 195 (0xc3)    monitorexit 	> Exit monitor for object
	// --  Implementa monitores para objetos (multithreading).
	void monitor(METHOD_DATA *, THREAD *, JVM *);
// EXTENDIDAS - Extended
	// 196 (0xc4)    wide 			> Extend local variable index by additional bytes
	// --  Modifica tamanho de argumentos de certas instrucoes aritmeticas.
	void wide_(METHOD_DATA *, THREAD *, JVM *);
	// 197 (0xc5)    multianewarray	> Create new multidimensional array (alocado na heap)
	// --  Criacao e manipulacao de array multidimensional.
	void createMultiArray(ARRAY *, int32_t *, uint8_t, uint8_t, JVM *);
	// 198 (0xc6)    ifnull 		> Branch if reference is null
	// 199 (0xc7)    ifnonnull 		> Branch if reference not null
	// --  Verifica se um referencia eh ou nao NULL e realiza um jump
	void ifNull(METHOD_DATA *, THREAD *, JVM *);
	// 200 (0xc8)    goto_w 		> Branch always (wide index)
	// 201 (0xc9)    jsr_w 			> Jump subroutine (wide index)
	// --  Jumps incondicionais de maior tamanho de offset
	void widejump(METHOD_DATA *, THREAD *, JVM *);
// RESERVADAS - Reserved
	// 202 (0xca)    breakpoint
	// --  Utilizado por debugadores para implementar breakpoints
	void breakpoint_(METHOD_DATA *, THREAD *, JVM *);
	// 203 (0xcb) a 253 (0xfd)
	// --  Faixa de opcodes nao utilizada pela Oracle
	void nonDefined(METHOD_DATA *, THREAD *, JVM *);
	// 254 (0xfe)    impdep1
	// 255 (0xff)    impdep2
	// --  Provem backdoors para implementacoes especificas
	void impdep(METHOD_DATA *, THREAD *, JVM *);



#endif // INTERPRETER_H

// -- https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-2.html#jvms-2.6.1
// 1.nop_
// 2.Tconst 			ok
// 3.Tipush 			ok
// 4.ldc_ 				-
// 5.Tload 				ok
// 6.Taload 			ok
// 7.Tstore 			ok
// 8.Tastore 			ok
// 9.handleStack 		-
// 10.Tadd 				ok
// 11.Tsub 				ok
// 12.Tmul 				ok
// 13.Tdiv 				ok
// 14.Trem 				ok
// 15.Tneg 				ok
// 16.Tshl 				ok
// 17.Tshr 				ok
// 18.Tushr 			ok
// 19.Tand 				ok
// 20.Tor 				ok
// 21.Txor 				ok
// 22.Tinc 				ok
// 23.i2T 				ok
// 24.l2T 				ok
// 25.f2T 				ok
// 26.d2T 				ok
// 27.Tcmp 				ok
// 28.TcmpOP 			ok (Tcmpl e Tcmpg)
// 29.ifOP 				-
// 30.if_icmOP 			ok (if_TcmpOP)
// 31.if_acmOP 			ok (if_TcmpOP)
// 32.jump 				-
// 33.switch_ 			-
// 34.Treturn 			ok
// 35.accessField 		-
// 36.invoke 			-
// 37.handleObject 		-
// 38.athrow 			-
// 39.properties 		-
// 40.monitor 			-
// 41.wide_ 			-
// 42.createMultiArray 	-
// 43.ifNull 			-
// 44.widejump 			-
// 45.breakpoint_ 		-
// 46.nonDefined 		-
// 47.impdep 			-

// not implemented
