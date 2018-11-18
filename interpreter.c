// interpreter.c

#include	"interpreter.h"
#include	"opcodes.h"
#include    <errno.h>

char* opcodesJVM[];

// var global para saber se extende operandos
int	isWide = 0;

#define MAX_INT 2147483647
#define MIN_INT -2147483648

//Chapter 7. Opcode Mnemonics by Opcode
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-7.html

// 2.11. Instruction Set Summary
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-2.html#jvms-2.11

//6.5. Instructions => AKI TEM UMA EXPLICAÇÃO DETALHADA DE CADA INSTRUÇÃO
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5

// INTERPRETADOR
// 2.11. Instruction Set Summary 
// do {
//     atomically calculate pc and fetch opcode at pc;
//     if (operands) fetch operands;
//     execute the action for the opcode;
// } while (there is more to do);
void interpreter(METHOD_DATA* method, THREAD* thread, JVM* jvm)
{
	thread->program_counter = method->bytecodes;

	// enquanto houver instruções
	while(thread->program_counter < (method->bytecodes + method->code_length))
	{	
		func[*thread->program_counter](method, thread, jvm);
	}
}

//-- instrucoes

// nop		0x00
// --  Faz nada.
// Apenas incrementa o contador do programa
void	nop_(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.nop
	thread->program_counter++;
}

// Tconst	0x01 a 0x0F
// --  Carregam constantes na pilha de operandos.
void	Tconst(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.aconst_null - Tconst_Tstore.class
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iconst_i - Tconst_Tstore.class
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lconst_l - Tconst_Tstore.class
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fconst_f - Tconst_Tstore.class
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dconst_d - Tconst_Tstore.class
	uint32_t* value = (uint32_t*) malloc(sizeof(uint32_t));
	float f = 0.0;
	
	switch(*thread->program_counter)
	{
		case aconst_null:
		case iconst_m1:
		case iconst_0:
		case iconst_1:
		case iconst_2:
		case iconst_3:
		case iconst_4:
		case iconst_5:
			if(*thread->program_counter == aconst_null){
				free(value);
				value = NULL;
			}
			else if(*thread->program_counter == iconst_m1){
				*value = -1;
			}
			else if(*thread->program_counter == iconst_0){
				*value = 0;
			}
			else if(*thread->program_counter == iconst_1){
				*value = 1;
			}
			else if(*thread->program_counter == iconst_2){
				*value = 2;
			}
			else if(*thread->program_counter == iconst_3){
				*value = 3;
			}
			else if(*thread->program_counter == iconst_4){
				*value = 4;
			}
			else if(*thread->program_counter == iconst_5){
				*value = 5;
			}

			if(!value){
				pushOperand(0, thread->jvm_stack);
			}
			else{
				pushOperand(*value, thread->jvm_stack);
			}
			break;
		case lconst_0:
		case lconst_1:
			*value = 0;
			if(*thread->program_counter == lconst_1){
				*value = 1;
			}
			// long exige dois push
			pushOperand(0, thread->jvm_stack);
			pushOperand(*value, thread->jvm_stack);
			break;
		case fconst_0:
		case fconst_1:
		case fconst_2:
			f = 0.0;
			if(*thread->program_counter == fconst_1){
				f = 1.0;
			}
			else if(*thread->program_counter == fconst_2){
				f = 2.0;
			}
			memcpy(value, &f, sizeof(uint32_t));
			pushOperand(*value, thread->jvm_stack);
			break;
		case dconst_0:
		case dconst_1:
			*value = 0x00000000;
			if(*thread->program_counter == dconst_1){
				*value = 0x3FF00000;
			}
			// double exige dois push
			pushOperand(*value, thread->jvm_stack);
			*value = 0x00000000;
			pushOperand(* value, thread->jvm_stack);
			break;
	}
	thread->program_counter++;
	if(value){
		free(value);
	}
}

// Tipush	0x10 e 0x11
// --  Carregam inteiros com sinal (16-short ou 8-byte) na pilha 
// --  de operandos.
void	Tipush(METHOD_DATA * method, THREAD * thread, JVM * jvm){ // TESTAR NEGATIVOS
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.bipush
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.sipush
	switch(*thread->program_counter)
	{
		case bipush:;
			int8_t aux1;
			aux1 = (int8_t) * (thread->program_counter + 1);
			pushOperand((uint32_t) aux1, thread->jvm_stack);
			break;
		case sipush:;
			uint8_t high;
			uint8_t low;
			int16_t aux2;
			high = *(thread->program_counter + 1);
			low = * (thread->program_counter + 2);
			aux2 = (high << 8) | low;

			#ifdef	DEBUG
			printf("\t%" PRId16, aux2);
			#endif

			pushOperand((uint32_t) aux2, thread->jvm_stack);

			// pula operando
			thread->program_counter++;
			break;
	}
	thread->program_counter	+= 2;
}
// ldc_		0x12 a 0x14
// --  Carregam constantes single (int, float, literal string) ou
// --  double word (long, double) na pilha de operandos.
// --  Tem como argumento um indice de 8 ou 16 bits da constant pool
void	ldc_(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ldc
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ldc_w
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ldc2_w
	uint32_t value;
	uint8_t high, low;
	uint16_t index;
	constant_pool_info * cp, * cp_aux;
	char * string;
	
	switch(*thread->program_counter)
	{
		case ldc:
		case ldc_w:
		case ldc2_w:
			if(*thread->program_counter == ldc){
				index = (uint16_t) * (thread->program_counter + 1);
			}else if(*thread->program_counter == ldc_w || *thread->program_counter == ldc2_w){
				high = * (thread->program_counter + 1);
				low = * (thread->program_counter + 2);

				index = (high << 8) | low;
				thread->program_counter++;
			}
			#ifdef	DEBUG_INSTRUCAO
			printf("\t#%" PRIuint8_t6, index);
			#endif

			cp = (thread->jvm_stack)->current_constant_pool + index - 1;
			switch(cp->tag)	
			{
				case (CONSTANT_Integer):

					#ifdef	DEBUG
					printf("\t<%" PRId32 ">", cp->u.Integer_Float.bytes);
					#endif
				// sem break, continua
				case (CONSTANT_Float):
					if(cp->tag == CONSTANT_Float){
						float aux;
						memcpy(&aux, &(cp->u.Integer_Float.bytes), sizeof(uint32_t));

						#ifdef	DEBUG
						printf("\t<%E>", aux);
						#endif

					}
					value = cp->u.Integer_Float.bytes;
					pushOperand(value, thread->jvm_stack);
					break;
				case (CONSTANT_String):
					cp_aux = (thread->jvm_stack)->current_constant_pool + cp->u.String.string_index - 1;
					string = (char *) cp_aux->u.Utf8.bytes;
					if(cp_aux->u.Utf8.length > 0)											
						string[cp_aux->u.Utf8.length] = '\0';

					#ifdef	DEBUG
					printf("\t<%s>", string);
					#endif

					// se for string da push no objeto string
					// e coloca objeto no topo da heap
					OBJECT 	*	object_string = NULL;
					object_string = (OBJECT *) malloc(sizeof(OBJECT));
					object_string->next = (jvm->heap_jvm)->objects;
					(jvm->heap_jvm)->objects = object_string;
					object_string->instance_variables = (VARIABLE *) malloc(sizeof(VARIABLE));
					(object_string->instance_variables)->value.u.InstanceReference.reference = (OBJECT *) string;
					(object_string->instance_variables)->next = NULL;
					pushOperand((uint32_t)(uintptr_t) object_string, thread->jvm_stack);
					break;
				case (CONSTANT_Long):

					#ifdef	DEBUG
					printf("\t<%" PRId64 ">", ((int64_t) cp->u.Long_Double.high_bytes << 32) | cp->u.Long_Double.low_bytes);
					#endif
					// sem break, continua 
				case (CONSTANT_Double):
					if(cp->tag == CONSTANT_Double){
						double	aux;
						uint64_t auxd = ((uint64_t) cp->u.Long_Double.high_bytes << 32) | cp->u.Long_Double.low_bytes;
						memcpy(&aux, &auxd, sizeof(double));

						#ifdef	DEBUG
						printf("\t<%E>", aux);
						#endif
					}
					value = cp->u.Long_Double.high_bytes;
					pushOperand(value, thread->jvm_stack);

					value = cp->u.Long_Double.low_bytes;
					pushOperand(value, thread->jvm_stack);
					break;
				default:
					printf("VerifyError: invalid index constant pool.\n");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
					break;
			}
			break;
	}
	thread->program_counter += 2;
}

// Tload	0x15 a 0x2D
// --  Carregam na pilha de operandos valores do vetor 
// --  de variaveis locais.
void	Tload(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iload
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lload
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fload
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dload
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.aload
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iload_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lload_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fload_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dload_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.aload_n

	uint16_t index;
	uint32_t value;
	switch(*thread->program_counter)
	{
		case iload:
		case lload:
		case fload:
		case dload:
		case aload:
			index = (uint16_t) *(thread->program_counter + 1);
			if (isWide)
			{
				index = index << 8;
				thread->program_counter++;
				index = index | *(thread->program_counter);
				isWide = 0;
			}
			#ifdef	DEBUG
			printf("\t%" PRIuint8_t6, index);
			#endif

			value = (thread->jvm_stack)->local_variables[index];
			pushOperand(value, thread->jvm_stack);

			if(*thread->program_counter == lload || *thread->program_counter == dload){
				value = (thread->jvm_stack)->local_variables[index + 1];
				pushOperand(value, thread->jvm_stack);
			}
			thread->program_counter++;
			break;
		case iload_0:
		case iload_1:
		case iload_2:
		case iload_3:
		case fload_0:
		case fload_1:
		case fload_2:
		case fload_3:
		case aload_0:
		case aload_1:
		case aload_2:
		case aload_3:
			if(*thread->program_counter == iload_0 ||
			*thread->program_counter == fload_0 ||
			*thread->program_counter == aload_0){
				index = 0;
			}
			else if(*thread->program_counter == iload_1 ||
			*thread->program_counter == fload_1 ||
			*thread->program_counter == aload_1){
				index = 1;
			}
			else if(*thread->program_counter == iload_2 ||
			*thread->program_counter == fload_2 ||
			*thread->program_counter == aload_2){
				index = 2;
			}
			else if(*thread->program_counter == iload_3 ||
			*thread->program_counter == fload_3 ||
			*thread->program_counter == aload_3){
				index = 3;
			}

			value = (thread->jvm_stack)->local_variables[index];
			pushOperand(value, thread->jvm_stack);

			break;
		case lload_0:
		case lload_1:
		case lload_2:
		case lload_3:
		case dload_0:
		case dload_1:
		case dload_2:
		case dload_3:
			if(*thread->program_counter == lload_0 ||
			 *thread->program_counter == dload_0){
				index = 0;
			}
			else if(*thread->program_counter == lload_1 ||
			 *thread->program_counter == dload_1){
				index = 1;
			}
			else if(*thread->program_counter == lload_2 ||
			 *thread->program_counter == dload_2){
				index = 2;
			}
			else if(*thread->program_counter == lload_3 ||
			 *thread->program_counter == dload_3){
				index = 3;
			}
			value = (thread->jvm_stack)->local_variables[index];
			pushOperand(value, thread->jvm_stack);

			value = (thread->jvm_stack)->local_variables[index + 1];
			pushOperand(value, thread->jvm_stack);

			break;
	}

	thread->program_counter++;
}

// Taload	0x2E a 0x35
// --  Carregam na pilha de operandos valores de arrays 
// --  referenciados pelo pilha.
void	Taload(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iaload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.laload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.faload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.daload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.aaload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.baload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.caload*/
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.saload*/
	uint32_t index;
	uint32_t * value = (uint32_t*) malloc(sizeof(uint32_t));
	
	// referencia de array
	ARRAY * arrayref;

	// get index from operand_stack
	index = popOperand(thread->jvm_stack);

	// get reference from operand_stack
	arrayref = (ARRAY *)(uintptr_t) popOperand(thread->jvm_stack);
	switch(*thread->program_counter)
	{
		case iaload:
			*value = (arrayref->entry)[index].u.Integer.integer;
			break;
		case faload:
			*value = (arrayref->entry)[index].u.Float.float_;
			break;
		case aaload:
			*value = (uint32_t)(uintptr_t) (arrayref->entry)[index].u.InstanceReference.reference;
			break;
		case baload:
			*value = (arrayref->entry)[index].u.Byte.byte;
			break;
		case caload:
			*value = (arrayref->entry)[index].u.Char.char_;
			break;
		case saload:
			*value = (arrayref->entry)[index].u.Short.short_;
			break;
		case laload:
			*value = (arrayref->entry)[index].u.Long.high_bytes;
			pushOperand(* value, thread->jvm_stack);
			*value = (arrayref->entry)[index].u.Long.low_bytes;
			break;
		case daload:
			*value = (arrayref->entry)[index].u.Double.high_bytes;
			pushOperand(* value, thread->jvm_stack);
			*value = (arrayref->entry)[index].u.Double.low_bytes;
			break;
	}
	pushOperand(* value, thread->jvm_stack);
	free(value);
	thread->program_counter++;
}

// Tstore	0x36 a 0x4E
// --  Grava valores do topo da pilha de operandos no vetor de variaveis locais.
void	Tstore(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.istore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lstore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fstore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dstore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.astore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.istore_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lstore_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fstore_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dstore_n
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.astore_n
	uint32_t high, low;
    switch(*thread->program_counter)
    {
        case istore:
        case fstore:
        case astore:{
            uint16_t index;
            uint32_t value;
            thread->program_counter++;
            index = (uint16_t) *(thread->program_counter);
            // get value from operand_stack
            value = popOperand(thread->jvm_stack);
            // store value into local variables
            (thread->jvm_stack)->local_variables[index] = value;

            thread->program_counter++;
            break;
        }
        case lstore:
        case dstore:{

            uint16_t index;

            thread->program_counter++;
            index = (uint16_t) *(thread->program_counter);
            // get high and low from operand_stack
            low = popOperand(thread->jvm_stack);
            high = popOperand(thread->jvm_stack);
            // store high and lowinto local variables
            (thread->jvm_stack)->local_variables[index] = high;
            (thread->jvm_stack)->local_variables[index + 1] = low;

            thread->program_counter++;
            break;
        }
        case istore_0:
        case istore_1:
        case istore_2:
        case istore_3:
        case fstore_0:
        case fstore_1:
        case fstore_2:
        case fstore_3:
        case astore_0:
        case astore_1:
        case astore_2:
        case astore_3:
        {
            uint16_t index;
            uint32_t value;

            if(*thread->program_counter == istore_0 || *thread->program_counter == fstore_0 || *thread->program_counter == astore_0){
                index = 0;
            }else if(*thread->program_counter == istore_1 || *thread->program_counter == fstore_1 || *thread->program_counter == astore_1){
                index = 1;
            }else if(*thread->program_counter == istore_2 || *thread->program_counter == fstore_2 || *thread->program_counter == astore_2){
                index = 2;
            }else if(*thread->program_counter == istore_3 || *thread->program_counter == fstore_3 || *thread->program_counter == astore_3){
                index = 3;
            }
            // get value from operand_stack
            value = popOperand(thread->jvm_stack);
            // store value into local variables
            (thread->jvm_stack)->local_variables[index] = value;

            thread->program_counter++;
            break;
        }
        case lstore_0:
        case lstore_1:
        case lstore_2:
        case lstore_3:
        case dstore_0:
        case dstore_1:
        case dstore_2:
        case dstore_3:{
            uint16_t index;

            if(*thread->program_counter == lstore_0 || *thread->program_counter == dstore_0){
                index = 0;
            }else if(*thread->program_counter == lstore_1 || *thread->program_counter == dstore_1){
                index = 1;
            }else if(*thread->program_counter == lstore_2 || *thread->program_counter == dstore_2){
                index = 2;
            }else if(*thread->program_counter == lstore_3 || *thread->program_counter == dstore_3){
                index = 3;
            }
            low = popOperand(thread->jvm_stack);
            high = popOperand(thread->jvm_stack);
            // store high and lowinto local variables
            (thread->jvm_stack)->local_variables[index] = high;
            (thread->jvm_stack)->local_variables[index + 1] = low;

            thread->program_counter++;
            break;
        }
    }
}

// Tastore	0x4F a 0x56
// --  Grava valores do topo da pilha de operandos em arrays referenciados pelo 
// --  vetor de variaveis locais.
void	Tastore(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iastore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lastore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fastore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dastore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.aastore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.bastore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.castore
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.sastore
	uint32_t index;
	uint32_t high, low, value;
	
	// referencia para array
	ARRAY * arrayref;

	// get value from operand_stack
	low = popOperand(thread->jvm_stack);
	if(*thread->program_counter == lastore || *thread->program_counter == dastore){
		high = popOperand(thread->jvm_stack);
	}
    value = low;
	// get index from operand_stack
	index = popOperand(thread->jvm_stack);

	// get reference from operand_stack
	arrayref = (ARRAY *)(uintptr_t) popOperand(thread->jvm_stack);

	switch(*thread->program_counter)
	{
		case	iastore:
			(arrayref->entry)[index].u.Integer.integer = (int32_t) value;
			break;
		case	fastore:
			(arrayref->entry)[index].u.Float.float_ = value;
			break;
		case	aastore:
			(arrayref->entry)[index].u.InstanceReference.reference = (OBJECT *)(uintptr_t) value;
			break;
		case	bastore:
			(arrayref->entry)[index].u.Byte.byte = (int8_t) value;
			break;
		case	castore:
			(arrayref->entry)[index].u.Char.char_ = (uint16_t) value;
			break;
		case	sastore:
			(arrayref->entry)[index].u.Short.short_ = (int16_t) value;
			break;
		case	lastore:
			(arrayref->entry)[index].u.Long.high_bytes = high;
			(arrayref->entry)[index].u.Long.low_bytes = low;
			break;
		case	dastore:
			(arrayref->entry)[index].u.Double.high_bytes = high;
			(arrayref->entry)[index].u.Double.low_bytes = low;
			break;
	}
	thread->program_counter++;
}

// *BUG NAS INSTRUÇÕES DUP2
// handleStack	0x57 a 0x5F	
// --  Para manipulacao da pilha de operandos.
// --  Descarte, duplicacao, troca de valores.
void	handleStack(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.pop
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.pop2
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dup
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dup_x1
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dup_x2
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dup2
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dup2_x1
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dup2_x2
	uint32_t value;
	uint32_t value1, value2, value3, value4;
	switch(*thread->program_counter)
	{
		case pop:
			value = popOperand(thread->jvm_stack);
			break;
		case pop2:
			value = popOperand(thread->jvm_stack);
			value = popOperand(thread->jvm_stack);
			break;
		case dup:
			// get value from operand_stack
			value = popOperand(thread->jvm_stack);

			//push value 2x
			pushOperand(value, thread->jvm_stack);
			pushOperand(value, thread->jvm_stack);
			break;
		case dup_x1:
			// get value from operand_stack
			value1 = popOperand(thread->jvm_stack);
			value2 = popOperand(thread->jvm_stack);

			//push value 1, 2, 1
			pushOperand(value1, thread->jvm_stack);
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			break;
		case dup_x2:
			// get value from operand_stack
			value1 = popOperand(thread->jvm_stack);
			value2 = popOperand(thread->jvm_stack);
			value3 = popOperand(thread->jvm_stack);

			//push value
			pushOperand(value1, thread->jvm_stack);
			pushOperand(value3, thread->jvm_stack);
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			break;
		case dup2:
			// get value from operand_stack
			value1 = popOperand(thread->jvm_stack);
			value2 = popOperand(thread->jvm_stack);

			//push value
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			break;
		case dup2_x1: // BUG SE FOR LONG OU DOUBLE
			// get value from operand_stack
			value1 = popOperand(thread->jvm_stack);
			value2 = popOperand(thread->jvm_stack);
			value3 = popOperand(thread->jvm_stack);

			//push value
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			pushOperand(value3, thread->jvm_stack);
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			break;
		case dup2_x2:	// BUG SE FOR LONG OU DOUBLE
			// get value from operand_stack
			value1 = popOperand(thread->jvm_stack);
			value2 = popOperand(thread->jvm_stack);
			value3 = popOperand(thread->jvm_stack);
			value4 = popOperand(thread->jvm_stack);

			//push value
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			pushOperand(value4, thread->jvm_stack);
			pushOperand(value3, thread->jvm_stack);
			pushOperand(value2, thread->jvm_stack);
			pushOperand(value1, thread->jvm_stack);
			break;
		case swap:
			// get value from operand_stack
			value1 = popOperand(thread->jvm_stack);
			value2 = popOperand(thread->jvm_stack);

			//push value
			pushOperand(value1, thread->jvm_stack);
			pushOperand(value2, thread->jvm_stack);
			break;
	}
	thread->program_counter++;
}

// *DOUBLE INCOMPLETO
// Tadd		0x60 a 0x63
// --  De adicao.
void	Tadd(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iadd
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ladd
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fadd
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dadd
	uint32_t	high, low, value;

	switch(*thread->program_counter) 
	{
		// INSTRUÇÃO IADD
		case iadd:;
			int32_t	first_int, second_int, total_int;
			// Desempilha operandos
			second_int = (int32_t) popOperand(thread->jvm_stack);
			first_int = (int32_t) popOperand(thread->jvm_stack);

			// Empilha soma
			total_int = first_int + second_int;
			pushOperand(total_int, thread->jvm_stack);
			break;
		case ladd:;
			int64_t	first_long, second_long, total_long;
			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_long = ((int64_t) high << 32) | low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_long = ((int64_t) high << 32) | low;

			total_long = first_long + second_long;
			high = (uint32_t) (total_long >> 32);
			low = (uint32_t) ((total_long << 32) >> 32);

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
		case fadd:;
			float	first_float, second_float, total_float;
			// Desempilha operandos
			value = popOperand(thread->jvm_stack);
			memcpy(&second_float, &value, sizeof(uint32_t));

			value = popOperand(thread->jvm_stack);
			memcpy(&first_float, &value, sizeof(uint32_t));

			total_float = first_float + second_float;
			memcpy(&value, &total_float, sizeof(uint32_t));

			// empilha operandos
			pushOperand(value, thread->jvm_stack);
			break;
		case dadd:;
			uint64_t	first_double, second_double, total_double;
/*			int32_t	sign1, sign2, sign_total;*/
/*			int32_t	exponent1, exponent2, exponent_total;*/
/*			int64_t	mantissa1, mantissa2, mantissa_total;*/
			double	first, second, total;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_double = ((uint64_t) high << 32) | low;

/*			sign1 = ((first_double >> 63) == 0) ? 1 : -1;*/
/*			exponent1 = ((first_double >> 52) & 0x7ffL);*/
/*			mantissa1 = (exponent1 == 0) ?*/
/*						(first_double & 0xfffffffffffffL) << 1 :*/
/*						(first_double & 0xfffffffffffffL) | 0x10000000000000L;*/

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_double = ((uint64_t) high << 32) | low;

/*			sign2 = ((second_double >> 63) == 0) ? 1 : -1;*/
/*			exponent2 = ((second_double >> 52) & 0x7ffL);*/
/*			mantissa2 = (exponent2 == 0) ?*/
/*						(second_double & 0xfffffffffffffL) << 1 :*/
/*						(second_double & 0xfffffffffffffL) | 0x10000000000000L;*/
/*			if(!mantissa1){*/
/*				total_double = second_double;*/
/*			}*/
/*			else if(!mantissa2){*/
/*				total_double = first_double;*/
/*			}*/
/*			else{*/
/*				// reduzir ao maior expoente*/
/*				// somar/subtrair as mantissas*/
/*				// normalizar o resultado*/
/*			}*/
			memcpy(&first, &first_double, 2*sizeof(uint32_t));
			memcpy(&second, &second_double, 2*sizeof(uint32_t));
			total = first + second;

			memcpy(&total_double, &total, 2* sizeof(uint32_t));
			high = total_double >> 32;
			low = total_double & 0xffffffff;
			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			break;
	}
	thread->program_counter++;
}

// Tsub		0x64 a 0x67
// --  De subtracao.		
void	Tsub(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.isub
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lsub
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fsub
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dsub
	uint32_t	high, low, value;

	switch(*thread->program_counter)
	{
		// INSTRUÇÃO ISUB
		case isub:;
			int32_t	first_int, second_int, total_int;
			// Desempilha operandos
			second_int = (int32_t) popOperand(thread->jvm_stack);
			first_int = (int32_t) popOperand(thread->jvm_stack);

			// Empilha diferenca
			total_int = first_int - second_int;
			pushOperand(total_int, thread->jvm_stack);
			break;
		case lsub:;
			int64_t	first_long, second_long, total_long;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_long = ((int64_t) high << 32) | low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_long = ((int64_t) high << 32) | low;

			total_long = first_long - second_long;
			high = (uint32_t) (total_long >> 32);
			low = (uint32_t) ((total_long << 32) >> 32);


			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
		case fsub:;
			float	first_float, second_float, total_float;
			// Desempilha operandos
			value = popOperand(thread->jvm_stack);
			memcpy(&second_float, &value, sizeof(uint32_t));

			value = popOperand(thread->jvm_stack);
			memcpy(&first_float, &value, sizeof(uint32_t));

			// Empilha diferenca
			total_float = first_float - second_float;
			memcpy(&value, &total_float, sizeof(uint32_t));
			pushOperand(value, thread->jvm_stack);
			break;
		case dsub:;
			uint64_t	first_double, second_double, total_double;
/*			int32_t	sign1, sign2, sign_total;*/
/*			int32_t	exponent1, exponent2, exponent_total;*/
/*			int64_t	mantissa1, mantissa2, mantissa_total;*/
			double	first, second, total;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_double = ((uint64_t) high << 32) | low;

/*			sign1 = ((first_double >> 63) == 0) ? 1 : -1;*/
/*			exponent1 = ((first_double >> 52) & 0x7ffL);*/
/*			mantissa1 = (exponent1 == 0) ?*/
/*						(first_double & 0xfffffffffffffL) << 1 :*/
/*						(first_double & 0xfffffffffffffL) | 0x10000000000000L;*/

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_double = ((uint64_t) high << 32) | low;

/*			sign2 = ((second_double >> 63) == 0) ? 1 : -1;*/
/*			exponent2 = ((second_double >> 52) & 0x7ffL);*/
/*			mantissa2 = (exponent2 == 0) ?*/
/*						(second_double & 0xfffffffffffffL) << 1 :*/
/*						(second_double & 0xfffffffffffffL) | 0x10000000000000L;*/
/*			if(!mantissa1){*/
/*				total_double = second_double;*/
/*				if(sign2 == 1){*/
/*					total_double |= 0x8000000000000000L;*/
/*				}*/
/*				else{*/
/*					total_double &= 0X7FFFFFFFFFFFFFFFL;*/
/*				}*/
/*			}*/
/*			else if(!mantissa2){*/
/*				total_double = first_double;*/
/*			}*/
/*			else{*/
/*				// reduzir ao maior expoente*/
/*				// somar/subtrair as mantissas*/
/*				// normalizar o resultado*/
/*			}*/
			memcpy(&first, &first_double, sizeof(uint64_t));
			memcpy(&second, &second_double, sizeof(uint64_t));
			total = first - second;
			memcpy(&total_double, &total, sizeof(uint64_t));
			high = total_double >> 32;
			low = total_double & 0xffffffff;
			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
	}
	thread->program_counter++;
}

// Tmul		0x68 a 0x6B
// --  De multiplicacao.
void	Tmul(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.imul
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lmul
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fmul
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dmul
	uint32_t high, low, value;

	switch(*thread->program_counter)
	{
		// INSTRUÇÃO IMUL
		case imul:;
			int32_t	first_int, second_int, total_int;
			// Desempilha operandos
			second_int = (int32_t) popOperand(thread->jvm_stack);
			first_int = (int32_t) popOperand(thread->jvm_stack);

			// Empilha produto
			total_int = first_int * second_int;
			// da lib errno.h, automatico
			if (errno == ERANGE)
			{
				puts("ArithmeticOverflowException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			pushOperand(total_int, thread->jvm_stack);
			break;
		case lmul:;
			int64_t	first_long, second_long, total_long;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_long = ((int64_t) high << 32) | low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_long = ((int64_t) high << 32) | low;

			total_long = first_long * second_long;
			high = (uint32_t) (total_long >> 32);
			low = (uint32_t) ((total_long << 32) >> 32);

            if (errno == ERANGE){
                puts("ArithmeticOverflowExeption");
                exitJVM(jvm);
                exit(EXIT_FAILURE);
			}
            /*if(first_long != 0 || second_long != 0){
                if(total_long/first_long != second_long){
                    puts("ArithmeticOverflowExeption");
                    exit(1);
                }
			}*/

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
		case fmul:;
			float	first_float, second_float, total_float;
			// Desempilha operandos
			value = popOperand(thread->jvm_stack);
			memcpy(&second_float, &value, sizeof(uint32_t));

			value = popOperand(thread->jvm_stack);
			memcpy(&first_float, &value, sizeof(uint32_t));
			total_float = first_float * second_float;
			
			if (errno == ERANGE){
                puts("ArithmeticOverflowExeption");
                exitJVM(jvm);
                exit(EXIT_FAILURE);
			}
			memcpy(&value, &total_float, sizeof(uint32_t));

            /*if(first_float != 0 || second_float != 0){
                if(total_float/first_float != second_float){
                    puts("ArithmeticOverflowExeption");
                    exit(1);
                }
			}*/

			// empilha operandos
			pushOperand(value, thread->jvm_stack);
			break;
		case dmul:;
			uint64_t	first_double, second_double, total_double;
/*			int32_t	sign1, sign2, sign_total;*/
/*			int32_t	exponent1, exponent2, exponent_total;*/
/*			int64_t	mantissa1, mantissa2, mantissa_total;*/
			double	first, second, total;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_double = ((uint64_t) high << 32) | low;

/*			sign1 = ((first_double >> 63) == 0) ? 1 : -1;*/
/*			exponent1 = ((first_double >> 52) & 0x7ffL);*/
/*			mantissa1 = (exponent1 == 0) ?*/
/*						(first_double & 0xfffffffffffffL) << 1 :*/
/*						(first_double & 0xfffffffffffffL) | 0x10000000000000L;*/

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_double = ((uint64_t) high << 32) | low;

/*			sign2 = ((second_double >> 63) == 0) ? 1 : -1;*/
/*			exponent2 = ((second_double >> 52) & 0x7ffL);*/
/*			mantissa2 = (exponent2 == 0) ?*/
/*						(second_double & 0xfffffffffffffL) << 1 :*/
/*						(second_double & 0xfffffffffffffL) | 0x10000000000000L;*/
/*			if(!mantissa1 || !mantissa2){*/
/*				total_double = 0;*/
/*			}*/
/*			else{*/
/*				// somar os expoentes*/
/*				// multiplicar as mantissas*/
/*				// normalizar o resultado*/
/*			}*/
			memcpy(&first, &first_double, sizeof(uint64_t));
			memcpy(&second, &second_double, sizeof(uint64_t));
			total = first * second;
            memcpy(&total_double, &total, sizeof(uint64_t));
			high = total_double >> 32;
			low = total_double & 0xffffffff;

            if (errno == ERANGE){
                puts("ArithmeticOverflowExeption");
                exitJVM(jvm);
                exit(EXIT_FAILURE);
			}
			/*if(first_double != 0 || second_double != 0){
                if(total_double/first_double != second_double){
                    puts("ArithmeticOverflowExeption");
                    exit(1);
                }
			}*/

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
	}
	thread->program_counter++;
}

// Tdiv		0x6C a 0x6F
// --  De divisao.
void	Tdiv(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.idiv
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ldiv
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fdiv
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ddiv
	uint32_t high, low, value;

	switch(*thread->program_counter)
	{
		// INSTRUÇÃO IMUL
		case	idiv:;
			int32_t	first_int, second_int, total_int;
			// Desempilha operandos
			second_int = (int32_t) popOperand(thread->jvm_stack);
			first_int = (int32_t) popOperand(thread->jvm_stack);

			// Empilha produto
			total_int = first_int / second_int;
			pushOperand(total_int, thread->jvm_stack);
			break;
		case	ldiv_:;
			int64_t	first_long, second_long, total_long;
			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_long = ((int64_t) high << 32) | low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_long = ((int64_t) high << 32) | low;

			total_long = first_long / second_long;
			high = (uint32_t) (total_long >> 32);
			low = (uint32_t) ((total_long << 32) >> 32);

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
		case fdiv:;
			float	first_float, second_float, total_float;
			// Desempilha operandos
			value = popOperand(thread->jvm_stack);
			memcpy(&second_float, &value, sizeof(uint32_t));

			value = popOperand(thread->jvm_stack);
			memcpy(&first_float, &value, sizeof(uint32_t));

			total_float = first_float / second_float;
			memcpy(&value, &total_float, sizeof(uint32_t));

			// empilha operandos
			pushOperand(value, thread->jvm_stack);
			break;
		case ddiv:;
			uint64_t	first_double, second_double, total_double;
/*			int32_t	sign1, sign2, sign_total;*/
/*			int32_t	exponent1, exponent2, exponent_total;*/
/*			int64_t	mantissa1, mantissa2, mantissa_total;*/
			double	first, second, total;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_double = ((uint64_t) high << 32) | low;

/*			sign1 = ((first_double >> 63) == 0) ? 1 : -1;*/
/*			exponent1 = ((first_double >> 52) & 0x7ffL);*/
/*			mantissa1 = (exponent1 == 0) ?*/
/*						(first_double & 0xfffffffffffffL) << 1 :*/
/*						(first_double & 0xfffffffffffffL) | 0x10000000000000L;*/

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_double = ((uint64_t) high << 32) | low;

/*			sign2 = ((second_double >> 63) == 0) ? 1 : -1;*/
/*			exponent2 = ((second_double >> 52) & 0x7ffL);*/
/*			mantissa2 = (exponent2 == 0) ?*/
/*						(second_double & 0xfffffffffffffL) << 1 :*/
/*						(second_double & 0xfffffffffffffL) | 0x10000000000000L;*/
/*			if(!mantissa2){*/
/*				puts("DivisionByZeroError");*/
/*				exit(EXIT_FAILURE);*/
/*			}*/
/*			else if(!mantissa2){*/
/*				total_double = 0;*/
/*			}*/
/*			else{*/
/*				// subtrair os expoentes*/
/*				// dividir as mantissas*/
/*				// normalizar o resultado*/
/*			}*/
			memcpy(&first, &first_double, sizeof(uint64_t));
			memcpy(&second, &second_double, sizeof(uint64_t));
			if(second == 0.0 || second == -0.0){
				puts("DivisionByZeroError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			total = first / second;
			memcpy(&total_double, &total, sizeof(uint64_t));
			high = total_double >> 32;
			low = total_double & 0xffffffff;
			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);
			break;
	}
	thread->program_counter++;
}

// Trem		0x70 a 0x73
// -- De resto.
void	Trem(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.irem
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lrem
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.frem
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.drem
    switch(*thread->program_counter)
    {
        case irem:
        {
            int32_t value1, value2;
            // get value from operand_stack
            value2 = (int32_t)popOperand(thread->jvm_stack);
            value1 =  (int32_t)popOperand(thread->jvm_stack);
            //push value
			pushOperand( (uint32_t)(value1 % value2), thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case lrem:
		{
			int64_t long_value1, long_value2;
			uint32_t high, low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			long_value2 = (int64_t) high;
			long_value2 <<= 32;
			long_value2 += low;

            low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			long_value1 = (int64_t) high;
			long_value1 <<= 32;
			long_value1 += low;

			long_value1 = long_value1 % long_value2;

			high = long_value1 >> 32;
			low = long_value1 & 0xffffffff;

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case frem:
		{
			float float_value1, float_value2;
			uint32_t value;

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value2, &value, sizeof(uint32_t));

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value1, &value, sizeof(uint32_t));

            float_value1 = fmodf( float_value1 , float_value2);

            memcpy(&value, &float_value1, sizeof(uint32_t));
			pushOperand(value, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case drem:
        {
            uint64_t value;
            uint32_t high, low;
            double double_value1, double_value2;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);

            value = (((uint64_t) high) << 32) + low;

            memcpy(&double_value2, &value, sizeof(uint64_t));

            low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);

            value = (((uint64_t) high) << 32) + low;

            memcpy(&double_value1, &value, sizeof(uint64_t));

            double_value1 = fmod( double_value1 , double_value2 );

            memcpy(&value, &double_value1, sizeof(uint64_t));

			high = value >> 32;
			low = value & 0xffffffff;

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
	}
}

// Tneg		0x74 a 0x77
// --  De negacao.
void	Tneg(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ineg
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lneg
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fneg
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dneg
    switch(*thread->program_counter)
    {
        case ineg:
        {
            int32_t value;
            // get value from operand_stack
            value = (int32_t) popOperand(thread->jvm_stack);
            value = -value;
            //push value
			pushOperand( (uint32_t)value, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case lneg:
		{
			int64_t long_value;
			uint32_t high, low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			long_value = (int64_t) high;
			long_value <<= 32;
			long_value += low;

			long_value = -long_value;

			high = long_value >> 32;
			low = long_value & 0xffffffff;

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case fneg:
		{
			float float_value;
			uint32_t value;

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value, &value, sizeof(uint32_t));

            float_value = -float_value;
            //copy bits from float_value into value
            memcpy(&value, &float_value, sizeof(uint32_t));

			pushOperand(value, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case dneg:
        {
            uint64_t double_value;
            uint64_t mask = 1;
            uint32_t high, low;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);

            double_value = (((uint64_t) high) << 32) + low;
            double_value |= (mask << 63);

			high = double_value >> 32;
			low = double_value & 0xffffffff;

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
	}
}

// Tshl		0x78 e 0x79
// --  De deslocamento a esquerda.
void	Tshl(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ishl
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lshl
	int32_t operand1, operand2, auxiliar2;
	int64_t auxiliar1;
	uint32_t auxuint32_t, aux2uint32_t;

	switch(*thread->program_counter) 
	{
		case ishl:
			operand2 = (int32_t) popOperand(thread->jvm_stack);
			operand1 = (int32_t) popOperand(thread->jvm_stack);
			operand2 = operand2 & 0x1f;
			pushOperand((operand1 << operand2),thread->jvm_stack);
			thread->program_counter++;
		break;
		case lshl:
			auxiliar2 	= (signed) popOperand(thread->jvm_stack);
			aux2uint32_t 		= popOperand(thread->jvm_stack);
			auxiliar1 	= (signed) popOperand(thread->jvm_stack);
			auxiliar1 	= auxiliar1 << 32;
			auxiliar1 	|= aux2uint32_t;
			auxiliar2 	= auxiliar2 & 0x3f;
			auxiliar1 	= auxiliar1 << auxiliar2;
			auxuint32_t = auxiliar1 >> 32;
			pushOperand(auxuint32_t, thread->jvm_stack);
			auxuint32_t = auxiliar1 & 0xffffffff;
			pushOperand(auxuint32_t, thread->jvm_stack);
			thread->program_counter++;
		break;
	}
}

// Tshr		0x7A e 0x7B
// --  De deslocamento a direita.
void	Tshr(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ishr
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lshr
	int32_t operand1, operand2, auxiliar2;
	int64_t auxiliar1;
	uint32_t auxuint32_t, aux2uint32_t;

	switch(*thread->program_counter) 
	{
		case ishr:
			operand2 = (int32_t) popOperand(thread->jvm_stack);
			operand1 = (int32_t) popOperand(thread->jvm_stack);
			operand2 = operand2 & 0x1f;
			pushOperand((operand1 >> operand2),thread->jvm_stack);
			thread->program_counter++;
		break;
		case lshr:
			auxiliar2 	= (signed) popOperand(thread->jvm_stack);
			aux2uint32_t 		= popOperand(thread->jvm_stack);
			auxiliar1 	= (signed) popOperand(thread->jvm_stack);
			auxiliar1 	= auxiliar1 << 32;
			auxiliar1 	|= aux2uint32_t;
			auxiliar2 	= auxiliar2 & 0x3f;
			auxiliar1 	= auxiliar1 >> auxiliar2;
			auxuint32_t = auxiliar1 >> 32;
			pushOperand(auxuint32_t, thread->jvm_stack);
			auxuint32_t 		= auxiliar1 & 0xffffffff;
			pushOperand(auxuint32_t, thread->jvm_stack);
			thread->program_counter++;
		break;
	}
}

// Tushr	0x7C e 0x7D
// --  De deslocamento a direita sem sinal.
void	Tushr(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iushr
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lushr
	int32_t operand1, operand2;
	int64_t auxiliar1, auxiliar2;
	uint32_t auxuint32_t, aux2uint32_t;

	switch(*thread->program_counter) 
	{
		case iushr:
			operand2 = (int32_t) popOperand(thread->jvm_stack);
			operand1 = (int32_t) popOperand(thread->jvm_stack);
			operand2 = operand2 & 0x1f;
			pushOperand((operand1 >> operand2),thread->jvm_stack);
			thread->program_counter++;
		break;
		case lushr:
			aux2uint32_t = popOperand(thread->jvm_stack);
			auxiliar2 = popOperand(thread->jvm_stack);
			auxiliar1 = popOperand(thread->jvm_stack);
			auxiliar1 = auxiliar1 << 32;
			auxiliar1 |= auxiliar2;
			aux2uint32_t = aux2uint32_t & 0x3f;
			auxiliar1 = auxiliar1 >> aux2uint32_t;
			auxuint32_t = auxiliar1 >> 32;
			pushOperand(auxuint32_t, thread->jvm_stack);
			auxuint32_t = auxiliar1 & 0xffffffff;
			pushOperand(auxuint32_t, thread->jvm_stack);
			thread->program_counter++;
		break;
	}
}


// Tand		0x7E e 0x7F
// --  De AND bit a bit.
void	Tand(METHOD_DATA * method, THREAD * thread, JVM * jvm){

	switch(*thread->program_counter) 
	{

		// INSTRUÇÃO IAND
		case iand:;
            uint32_t value;
            int32_t first_operand, second_operand, result;
			// Desempilha operando
            value = popOperand(thread->jvm_stack);
            second_operand = (int32_t) value;
			// Desempilha operando
            value = popOperand(thread->jvm_stack);
			first_operand = (int32_t) value;
			// Empilha resultado
            result = first_operand & second_operand;
            pushOperand(result, thread->jvm_stack);
			thread->program_counter++;
			break;

		//Intrução LAND
		case land:;
            uint32_t low1,high1, low2, high2;

            low2 = popOperand(thread->jvm_stack);
            high2 = popOperand(thread->jvm_stack);

            low1 = popOperand(thread->jvm_stack);
            high1 = popOperand(thread->jvm_stack);

            pushOperand((high1 & high2), thread->jvm_stack);
            pushOperand((low1 & low2), thread->jvm_stack);

            thread->program_counter++;
            break;
    }
}

// Tor		0x80 e 0x81
// --  De OR bit a bit.
void	Tor(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ior
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lor
	switch(*thread->program_counter) 
	{
		case ior:;
			uint32_t	first_op, second_op;

			first_op = popOperand(thread->jvm_stack);
			second_op = popOperand(thread->jvm_stack);
			pushOperand((first_op | second_op), thread->jvm_stack);
			break;
		case lor:;
            uint32_t low1,high1, low2, high2;

            low2 = popOperand(thread->jvm_stack);
            high2 = popOperand(thread->jvm_stack);

            low1 = popOperand(thread->jvm_stack);
            high1 = popOperand(thread->jvm_stack);

            pushOperand((high1 | high2), thread->jvm_stack);
            pushOperand((low1 | low2), thread->jvm_stack);

            break;
	}
	thread->program_counter++;
}

// Txor		0x82 e 0x83
// --  De XOR bit a bit.
void	Txor(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ixor
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lxor
	switch(*thread->program_counter) 
	{
		case ixor:;
			uint32_t	first_op, second_op;

			second_op = popOperand(thread->jvm_stack);
			first_op = popOperand(thread->jvm_stack);
			pushOperand((first_op ^ second_op), thread->jvm_stack);
			break;
		case lxor:;
			uint32_t low1,high1, low2, high2;

            low2 = popOperand(thread->jvm_stack);
            high2 = popOperand(thread->jvm_stack);

            low1 = popOperand(thread->jvm_stack);
            high1 = popOperand(thread->jvm_stack);

            pushOperand((high1 ^ high2), thread->jvm_stack);
            pushOperand((low1 ^ low2), thread->jvm_stack);

            break;
	}
	thread->program_counter++;
}

// Tinc		0x84
// --  De incremento de constante a posicao no array de variaveis locais.
void	Tinc(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iinc
	uint8_t	index = (uint8_t) *(thread->program_counter + 1);
	int8_t	const_ = (int8_t) *(thread->program_counter + 2);
	
	(thread->jvm_stack)->local_variables[index] += (int32_t) const_;
	thread->program_counter += 3;
}

// i2T		0x85 a 0x87; 0x91 a 0x93
// --  Convertem de inteiro pra um outro tipo.
void	i2T(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.i2l
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.i2f
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.i2d
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.i2b
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.i2c
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.i2s
	switch(*thread->program_counter)
	{
		case i2l:
		{
			int32_t value, high, low;
			int64_t long_value;

			value = (int32_t) popOperand(thread->jvm_stack);
            long_value = (int64_t) value;

			high = long_value >> 32;
			low = long_value & 0xffffffff;

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case i2f:
		{
			float float_value;
			int32_t value;

            value = (int32_t) popOperand(thread->jvm_stack);
            float_value = (float) value;
            //copy bits from float_value into value
            memcpy(&value, &float_value, sizeof(uint32_t));

			pushOperand(value, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case i2d:
        {
            uint64_t double_value = 0;
            int32_t value,high, low;

            value = (int32_t) popOperand(thread->jvm_stack);

            double  double_ = (double) value;
            memcpy(&double_value, &double_, sizeof(uint64_t));

			high = double_value >> 32;
			low = double_value & 0xffffffff;

			pushOperand(high, thread->jvm_stack);
			pushOperand(low, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case i2b:
		{
			int32_t value;
			value = (int32_t)((int8_t) popOperand(thread->jvm_stack));

			pushOperand((uint32_t)value, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
		case i2c:
		{
            int32_t value;
			value = (int32_t)((uint16_t) popOperand(thread->jvm_stack));

			pushOperand((uint32_t)value, thread->jvm_stack);

			thread->program_counter++;
            break;
		}
		case i2s:
		{
			int32_t value;
			value = (int32_t)((int16_t) popOperand(thread->jvm_stack));

			pushOperand((uint32_t)value, thread->jvm_stack);

			thread->program_counter++;
			break;
		}
	}
}

// l2T		0x88 a 0x8A
// --  Convertem de long para um outro tipo.
void	l2T(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.l2i
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.l2f
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.l2d
	switch(*thread->program_counter)
	{
		case l2i:
		{
			uint32_t high, low;
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			high++;
			pushOperand(low, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case l2f:
        {
            int64_t long_value;
            int64_t value;
            uint32_t high, low, aux;
            float float_value;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);

            value = (((int64_t) high) << 32) + low;
            long_value = (int64_t) value;
            float_value = (float) long_value;

			memcpy(&aux, &float_value, sizeof(uint32_t));

			pushOperand(aux, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case l2d:
        {
            uint64_t double_value = 0;
            int64_t value;
            uint32_t high, low;

            low = popOperand(thread->jvm_stack);
            high = popOperand(thread->jvm_stack);

            value = ((int64_t)(high) << 32) + low;

            double  double_ = (double) value;

            memcpy(&double_value, &double_, sizeof(uint64_t));

            high = double_value >> 32;
            low = double_value & 0xffffffff;

            pushOperand(high, thread->jvm_stack);
            pushOperand(low, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
    }
}

// *REVER
// f2T		0x8B a 0x8D
// --  Convertem de double para um outro tipo.
void	f2T(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.f2i
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.f2l
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.f2d
    switch(*thread->program_counter)
    {
        case f2i:
        {
            uint32_t value;
            float float_value;

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value, &value, sizeof(uint32_t));

            value = (uint32_t)((int32_t)float_value);
            pushOperand(value, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case f2l:
        {//rever
            uint32_t value;
            uint64_t long_value;
            float float_value;

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value, &value, sizeof(uint32_t));

            long_value = (uint64_t) ((int64_t)float_value);
            pushOperand(long_value, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
        case f2d:
        {
            uint64_t double_value = 0;
            uint32_t value,high, low;
            float float_value;

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value, &value, sizeof(uint32_t));

            double  double_ = (double) float_value;
            memcpy(&double_value, &double_, sizeof(uint64_t));

			high = double_value >> 32;
            low = double_value & 0xffffffff;

            pushOperand(high, thread->jvm_stack);
            pushOperand(low, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
    }
}

// d2T		0x8E a 0x90
// --  Convertem de double para um outro tipo.
void	d2T(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.d2i
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.d2l
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.d2f
	switch(*thread->program_counter)
    {
        case d2i:
        case d2l:
        case d2f:
        {
            int64_t value;
            uint32_t high, low;
            int32_t s, e;
            int64_t m;

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);

			value = (((int64_t) high) << 32) + low;
			switch(value)
			{
				case 0x7ff0000000000000L:
					//printf("\n\t\tDouble:\t\t\t+∞\n\n");
					break;
				case 0xfff0000000000000L:
					//printf("\n\t\tDouble:\t\t\t-∞\n\n");
					break;
				default:
					if((value >= 0x7ff0000000000001L && value <= 0x7ffffffffffffL) ||
					(value >= 0xfff0000000000001L && value <= 0xffffffffffffffffL )){
						//printf("\n\t\tDouble:\t\t\tNaN\n\n");
					}
					else{
						s = ((value >> 63) == 0) ? 1 : -1;
						e = ((value >> 52) & 0x7ffL);
						m = (e == 0) ?
						(value & 0xfffffffffffffL) << 1 :
						(value & 0xfffffffffffffL) | 0x10000000000000L;
						//printf("\n\t\tDouble:\t\t\t%f\n\n", (double) s*m*pow(2, (e-1075)));
					}
			}

            if(*thread->program_counter == d2i)
            {
                int32_t int_value;

                int_value = (int32_t)(s*m*pow(2, (e-1075)));
                pushOperand((uint32_t)int_value, thread->jvm_stack);

            }else if(*thread->program_counter == d2l)
            {
                uint64_t long_value = (uint64_t)((int64_t)s*m*pow(2, (e-1075)));

                uint32_t high = long_value >> 32;
                uint32_t low = long_value & 0xffffffff;

                pushOperand(high, thread->jvm_stack);
                pushOperand(low, thread->jvm_stack);
            }else if(*thread->program_counter == d2f)
            {
                float float_value;
                uint32_t value;

                float_value = s*m*pow(2, (e-1075));

                memcpy(&value, &float_value, sizeof(uint32_t));

                pushOperand(value, thread->jvm_stack);
            }
            thread->program_counter++;
            break;
        }
    }
}

// Tcmp		0x94
// -- Comparacao de tipo integral (long).
void	Tcmp(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lcmp
    uint64_t value1, value2;
    uint32_t high, low;
    int32_t result;

    low = popOperand(thread->jvm_stack);
    high = popOperand(thread->jvm_stack);

    value2 = (((uint64_t) high) << 32) + low;

    low = popOperand(thread->jvm_stack);
    high = popOperand(thread->jvm_stack);

    value1 = (((uint64_t) high) << 32) + low;

	if ( value1 == value2 ){
        result = 0;
	}else if ( value1 > value2 ){
        result = 1;
	}
	else{
        result = -1;
	}

	pushOperand((uint32_t)result, thread->jvm_stack);

	thread->program_counter++;
}

// *REVER
// TcmpOP	0x95 a 0x98
// --  Comparacao de ponto flutuante.
void	TcmpOP(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fcmpl
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.fcmpg
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dcmpl
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dcmpg
	switch(*thread->program_counter)
    {
        case fcmpl:
        case fcmpg:
        {
            int32_t result;
            uint32_t value;
            float float_value1, float_value2;

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value2, &value, sizeof(uint32_t));

            value = popOperand(thread->jvm_stack);
            memcpy(&float_value1, &value, sizeof(uint32_t));

            if ( float_value1 == float_value2 ){
                result = 0;
            }else if ( float_value1 > float_value2 ){
                result = 1;
            }
            else{
                result = -1;
            }
            // se NaN

            pushOperand((uint32_t)result, thread->jvm_stack);

            thread->program_counter++;
            break;
        }

        case dcmpl:
        case dcmpg:
        {
            uint64_t	first_double, second_double;
			double double_value1, double_value2;
			int32_t result;
			uint32_t low, high;

			// desempilha operandos
			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			second_double = ((uint64_t) high << 32) | low;
            memcpy(&double_value2, &second_double, sizeof(uint64_t));

			low = popOperand(thread->jvm_stack);
			high = popOperand(thread->jvm_stack);
			first_double = ((uint64_t) high << 32) | low;
            memcpy(&double_value1, &first_double, sizeof(uint64_t));

			if ( double_value1 == double_value2 ){
                result = 0;
            }else if ( double_value1 > double_value2 ){
                result = 1;
            }
            else{
                result = -1;
            }
            // se NaN

            pushOperand((uint32_t)result, thread->jvm_stack);

            thread->program_counter++;
            break;
        }
    }
}

// ifOP		0x99 a 0x9E
// --  Compara valor inteiro com zero e realiza um jump.
void	ifOP(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifeq
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifne
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.iflt
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifge
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifgt
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifle
    switch(*thread->program_counter)
    {
        case ifeq:
        case ifne:
        case iflt:
        case ifge:
        case ifgt:
        case ifle:
        {
            uint8_t	branchbyte1 = * (thread->program_counter + 1);
            uint8_t	branchbyte2 = * (thread->program_counter + 2);
            int16_t	branch = (branchbyte1 << 8) | branchbyte2;

            int32_t value = (int32_t)popOperand(thread->jvm_stack);

            if(*thread->program_counter == ifeq)
            {
                if(value == 0){
                    thread->program_counter += branch;
                }else{
                    thread->program_counter += 3;
                }
            }else if(*thread->program_counter == ifne)
            {
                if(value != 0){
                    thread->program_counter += branch;
                }else{
                    thread->program_counter += 3;
                }
            }else if(*thread->program_counter == iflt)
            {
                if(value < 0){
                    thread->program_counter += branch;
                }else{
                    thread->program_counter += 3;
                }
            }else if(*thread->program_counter == ifge)
            {
                if(value >= 0){
                    thread->program_counter += branch;
                }else{
                    thread->program_counter += 3;
                }
            }else if(*thread->program_counter == ifgt)
            {
                if(value > 0){
                    thread->program_counter += branch;
                }else{
                    thread->program_counter += 3;
                }
            }else if(*thread->program_counter == ifle)
            {
                if(value <= 0){
                    thread->program_counter += branch;
                }else{
                    thread->program_counter += 3;
                }
            }

            break;
        }
    }
}

// if_icmOP	0x9F a 0xA4
// --  Compara dois valores inteiros e realiza um jump.
void	if_icmOP(METHOD_DATA * method, THREAD * thread, JVM * jvm){
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_icmpeq
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_icmpne
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_icmplt
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_icmpge
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_icmpgt
//https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_icmple
	uint8_t	branchbyte1 = * (thread->program_counter + 1);
	uint8_t	branchbyte2 = * (thread->program_counter + 2);
	int16_t	branch = (branchbyte1 << 8) | branchbyte2;

	#ifdef	DEBUG
	printf("\t%" PRId16 "\t(%+" PRId16 ")", (uint16_t) (thread->program_counter - method->bytecodes) + branch, branch);
	#endif

	int32_t	value2	= (int32_t) popOperand(thread->jvm_stack);
	int32_t	value1	= (int32_t) popOperand(thread->jvm_stack);

	switch(* thread->program_counter)
	{
		case	if_icmpeq:
			if(value1 == value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
        case	if_icmpne:
			if(value1 != value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
        case	if_icmplt:
			if(value1 < value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
		case	if_icmpge:
			if(value1 >= value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
        case	if_icmpgt:
			if(value1 > value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
        case	if_icmple:
			if(value1 <= value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
	}
}

// if_acmOP	0xA5 e 0xA6
// --  Compara dois valores do tipo reference.
void	if_acmOP(METHOD_DATA * method, THREAD * thread, JVM * jvm){
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_acmpeq
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.if_acmpne
    uint8_t	branchbyte1 = * (thread->program_counter + 1);
	uint8_t	branchbyte2 = * (thread->program_counter + 2);
	int16_t	branch = (branchbyte1 << 8) | branchbyte2;

	#ifdef	DEBUG
	printf("\t%" PRId16 "\t(%+" PRId16 ")", (uint16_t) (thread->program_counter - method->bytecodes) + branch, branch);
	#endif

	int32_t	value2	= (int32_t) popOperand(thread->jvm_stack);
	int32_t	value1	= (int32_t) popOperand(thread->jvm_stack);

	switch(* thread->program_counter){
		case	if_acmpeq:
			if(value1 == value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
        case	if_acmpne:
			if(value1 != value2){
				thread->program_counter += branch;
			}else{
                thread->program_counter += 3;
			}
			break;
	}
}

// aqui
// jump		0xA7 a 0xA9
//	jumps incondicionais
void	jump(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.goto*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.jsr*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ret*/
    uint8_t	branchbyte1 = * (thread->program_counter + 1);
	uint8_t	branchbyte2 = * (thread->program_counter + 2);
	int16_t	branch = (branchbyte1 << 8) | branchbyte2;

	switch(* thread->program_counter){
		case	goto_:
            thread->program_counter += branch;
			break;
        case	jsr:
            pushOperand((uint32_t)(uintptr_t) (thread->program_counter += 3), thread->jvm_stack);
            thread->program_counter += branch;
			break;
        case	ret:{
            thread->program_counter++;
            uint16_t index = (uint16_t)*(thread->program_counter);

            if (isWide == 1){
                thread->program_counter++;
                index = (index << 8) | *(thread->program_counter);
                isWide = 0;
            }

            *thread->program_counter = (thread->jvm_stack)->local_variables[index];
			break;
        }
	}
}

// switch_	0xAA e 0xAB
//	jump table
void	switch_(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.tableswitch*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lookupswitch*/
	uint8_t	* backupPC = thread->program_counter;

    // procura o primeiro endereco divisivel por 4
    while (((uintptr_t) (thread->program_counter + 1)) % 4){
        thread->program_counter++;
    }

	uint8_t	defaultbyte1 = * (++thread->program_counter);
	uint8_t	defaultbyte2 = * (++thread->program_counter);
	uint8_t	defaultbyte3 = * (++thread->program_counter);
	uint8_t	defaultbyte4 = * (++thread->program_counter);
	int32_t	default_ = ((defaultbyte1 << 24) | ((defaultbyte2 << 16) | ((defaultbyte3 << 8) | defaultbyte4)));

   

	switch(* backupPC){
		case	tableswitch:;
			uint8_t	lowbyte1 = * (++thread->program_counter);
			uint8_t	lowbyte2 = * (++thread->program_counter);
			uint8_t	lowbyte3 = * (++thread->program_counter);
			uint8_t	lowbyte4 = * (++thread->program_counter);
			int32_t	low = ((lowbyte1 << 24) | ((lowbyte2 << 16) | ((lowbyte3 << 8) | lowbyte4)));

            #ifdef  DEBUG_INSTRUCAO
            printf("\tlow = %" PRId32 "\n", low);
            #endif

			uint8_t	highbyte1 = * (++thread->program_counter);
			uint8_t	highbyte2 = * (++thread->program_counter);
			uint8_t	highbyte3 = * (++thread->program_counter);
			uint8_t	highbyte4 = * (++thread->program_counter);
			int32_t	high = ((highbyte1 << 24) | ((highbyte2 << 16) | ((highbyte3 << 8) | highbyte4)));

            #ifdef  DEBUG_INSTRUCAO
    printf("\thigh = %" PRId32 "\n", high);
    #endif

			if(low > high){
				puts("VerifyError: invalid tableswitch arguments");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			uint32_t	num_jumps = high - low + 1;
			uint32_t	* jump_table = (uint32_t *) malloc(num_jumps * sizeof(uint32_t));
			for(uint32_t i = 0; i < num_jumps; i++){
			    highbyte1 = * (++thread->program_counter);
                highbyte2 = * (++thread->program_counter);
                highbyte3 = * (++thread->program_counter);
                highbyte4 = * (++thread->program_counter);

				jump_table[i] = ((highbyte1 << 24) | ((highbyte2 << 16) | ((highbyte3 << 8) | highbyte4)));
	
			}
			int32_t	index = popOperand(thread->jvm_stack);

              

			if((index < low) || (index > high)){
				thread->program_counter = backupPC + default_;
			}
			else{
				thread->program_counter = backupPC + jump_table[index - low];
			}

			free(jump_table);
			break;
		case	lookupswitch:;
			uint8_t	npairint8_t = * (++thread->program_counter);
			uint8_t	npairint16_t = * (++thread->program_counter);
			uint8_t	npairs3 = * (++thread->program_counter);
			uint8_t	npairint32_t = * (++thread->program_counter);
			int32_t	npairs = ((npairint8_t << 24) | ((npairint16_t << 16) | ((npairs3 << 8) | npairint32_t)));

			if(npairs < 0){
				puts("VerifyError: invalid lookupswitch arguments");
			}

			int32_t	* match_table = (int32_t *) malloc(npairs * sizeof(int32_t));
			int32_t	* offset_table = (int32_t *) malloc(npairs * sizeof(int32_t));
			for(int32_t i = 0; i < npairs; i++){
				uint8_t	matchbyte1 = * (++thread->program_counter);
				uint8_t	matchbyte2 = * (++thread->program_counter);
				uint8_t	matchbyte3 = * (++thread->program_counter);
				uint8_t	matchbyte4 = * (++thread->program_counter);
				int32_t	match = ((matchbyte1 << 24) | ((matchbyte2 << 16) | ((matchbyte3 << 8) | matchbyte4)));
				match_table[i] = match;

				uint8_t	offsetbyte1 = * (++thread->program_counter);
				uint8_t	offsetbyte2 = * (++thread->program_counter);
				uint8_t	offsetbyte3 = * (++thread->program_counter);
				uint8_t	offsetbyte4 = * (++thread->program_counter);
				int32_t	offset = ((offsetbyte1 << 24) | ((offsetbyte2 << 16) | ((offsetbyte3 << 8) | offsetbyte4)));
				offset_table[i] = offset;
			}

			int32_t	key = (int32_t) popOperand(thread->jvm_stack);

			uint32_t	index_match = 0;
			bool	find_index = false;

			while(!find_index && index_match < npairs ){
				if(match_table[index_match] == key){
					find_index = true;
				}
				else{
                    index_match++;
				}
			}

			if(find_index){
				thread->program_counter = backupPC + offset_table[index_match];
			}
			else{
			    thread->program_counter = backupPC + default_;
			}
			free(match_table);
			free(offset_table);
			break;
	}
}

// Treturn	0xAC a 0xB1
//	retorna valor do metodo e retoma a execuçao do método invocador
void	Treturn(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ireturn*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.lreturn*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.freturn*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.dreturn*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.areturn*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.return*/
	FRAME	*	aux = thread->jvm_stack;
	uint32_t	value, value_h, value_l;
	switch(* thread->program_counter){// PARA TESTAR O HELLOWORLD
		case	ireturn:
		case	freturn:
		case	areturn:
			value = popOperand(thread->jvm_stack);
			// retorna a execução ao método invocador;
			thread->jvm_stack = (thread->jvm_stack)->next;
			unloadFrame(aux);
			pushOperand(value, thread->jvm_stack);
			break;
		case	lreturn:
		case	dreturn:
			value_l = popOperand(thread->jvm_stack);
			value_h = popOperand(thread->jvm_stack);
			// retorna a execução ao método invocador;
			thread->jvm_stack = (thread->jvm_stack)->next;
			unloadFrame(aux);
			pushOperand(value_h, thread->jvm_stack);
			pushOperand(value_l, thread->jvm_stack);
			break;
		case	return_:;
			// retorna a execução ao método invocador;
			FRAME	*	aux = thread->jvm_stack;
			thread->jvm_stack = (thread->jvm_stack)->next;
			unloadFrame(aux);
			break;
	}
	thread->program_counter += method->code_length;
}

// accessField	0xB2 a 0xB5
//	manipula fields
void	accessField(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.getstatic*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.putstatic*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.getfield*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.putfield*/

	if((method->bytecodes + method->code_length - 2) <= thread->program_counter){
		puts("VerifyError: instrução sem a quantidade de argumentos correta.");
		exitJVM(jvm);
		exit(EXIT_FAILURE);
	}

	uint8_t	indexbyte1 = *(thread->program_counter + 1);
	uint8_t	indexbyte2 = *(thread->program_counter + 2);
	uint16_t	index = (indexbyte1 << 8) | indexbyte2;

	OBJECT	* objectref;


	// RESOLUÇÃO DO FIELD
		// Nome da classe do field
	constant_pool_info	* cp_aux = (thread->jvm_stack)->current_constant_pool;
	cp_aux = cp_aux + index - 1; // falta verificar se o indice está nos limites da constant pool
	cp_aux = (thread->jvm_stack)->current_constant_pool + cp_aux->u.Ref.name_index - 1;
	constant_pool_info	* cp_class_name = (thread->jvm_stack)->current_constant_pool + cp_aux->u.Class.name_index - 1;
	char	* class_name = (char *) cp_class_name->u.Utf8.bytes;
	class_name[cp_class_name->u.Utf8.length] = '\0';



		// nome do field
	cp_aux = (thread->jvm_stack)->current_constant_pool;
	cp_aux = cp_aux + index - 1;
	cp_aux = (thread->jvm_stack)->current_constant_pool + cp_aux->u.Ref.name_and_type_index - 1;
	constant_pool_info	* cp_field_name = (thread->jvm_stack)->current_constant_pool + cp_aux->u.NameAndType.name_index - 1;
	char	* field_name = (char *) cp_field_name->u.Utf8.bytes;
	field_name[cp_field_name->u.Utf8.length] = '\0';

		// descritor do field
	constant_pool_info * cp_field_descriptor = (thread->jvm_stack)->current_constant_pool + cp_aux->u.NameAndType.descriptor_index - 1;
	char	* field_descriptor = (char *) cp_field_descriptor->u.Utf8.bytes;
	field_descriptor[cp_field_descriptor->u.Utf8.length] = '\0';


	// CONTROLE DE ACESSO
	uint8_t	* backupPC = thread->program_counter;
	CLASS_DATA	* field_class = getClass(cp_class_name, jvm);
	if(!field_class){// se a classe do field não foi carregada

		

		char	* string = malloc((strlen(class_name) + 7) * sizeof(CHAR));
		strcpy(string, class_name);
		strcat(string, ".class");


		loadClass(string, &field_class, method->CLASS_DATA, jvm);
		free(string);
		linkClass(field_class, jvm);
		initializeClass(field_class, jvm, thread);
		thread->program_counter = backupPC;

		#ifdef	DEBUG_METODO
		puts("\n=======================");
		printf("Resume\t");
		PrintConstantUtf8((method->CLASS_DATA)->class_name, stdout);
		char	* method_name = (method->method_name)->u.Utf8.bytes;
		method_name[(method->method_name)->u.Utf8.length] = '\0';
		printf(".%s\n", method_name);
		puts("=======================");
		#endif

	}
	else{
		if(field_class != method->CLASS_DATA){// Se o Field não for da mesma classe
			// verifica se a classe do field é acessível pelo método corrente
			if(!(field_class->modifiers & ACC_PUBLIC) &
			(field_class->class_loader_reference != (method->CLASS_DATA)->class_loader_reference)){
				puts("IllegalAccessError: acesso indevido à classe ou interface.");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
		}
	}

	VARIABLE	* var;
	switch(* thread->program_counter){
		case	getstatic:;
			var = getClassVariable(cp_field_name, field_class);
			if(!var){
				puts("NoSuchFieldError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			switch((var->value).type){
				case	BYTE:
					pushOperand((int32_t)(uintptr_t) (var->value).u.Byte.byte, thread->jvm_stack);
					break;
				case	CHAR:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.Char.char_, thread->jvm_stack);
					break;
				case	FLOAT:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.Float.float_, thread->jvm_stack);
					break;
				case	INT:
					pushOperand((int32_t)(uintptr_t) (var->value).u.Integer.integer, thread->jvm_stack);
					break;
				case	REF_INST:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.ArrayReference.reference, thread->jvm_stack);
					break;
				case	SHORT:
					pushOperand((int32_t)(uintptr_t) (var->value).u.Short.short_, thread->jvm_stack);
					break;
				case	BOOLEAN:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.Boolean.boolean, thread->jvm_stack);
					break;
				case	REF_ARRAY:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.InstanceReference.reference, thread->jvm_stack);
					break;
				case	DOUBLE:
				case	LONG:;
					if((var->value).type == DOUBLE){
						pushOperand((uint32_t) (var->value).u.Double.high_bytes, thread->jvm_stack);
						pushOperand((uint32_t) (var->value).u.Double.low_bytes, thread->jvm_stack);
					}
					else{
						pushOperand((uint32_t) (var->value).u.Long.high_bytes, thread->jvm_stack);
						pushOperand((uint32_t) (var->value).u.Long.low_bytes, thread->jvm_stack);
					}
					break;
				default:
					puts("VerifyError: descritor de field inválido");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
			}
			break;
		case	putstatic:
			var = getClassVariable(cp_field_name, field_class);
			if(!var){
				puts("NoSuchFieldError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			switch((var->value).type){
				case	BYTE:
					(var->value).u.Byte.byte = (int8_t) popOperand(thread->jvm_stack);
					break;
				case	CHAR:
					(var->value).u.Char.char_ = (uint8_t) popOperand(thread->jvm_stack);
					break;
				case	FLOAT:
					(var->value).u.Float.float_ = popOperand(thread->jvm_stack);
					break;
				case	INT:
					(var->value).u.Integer.integer = (int32_t) popOperand(thread->jvm_stack);
					break;
				case	REF_INST:
					(var->value).u.InstanceReference.reference = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
					break;
				case	SHORT:
					(var->value).u.Short.short_ = (int16_t) popOperand(thread->jvm_stack);
					break;
				case	BOOLEAN:
					(var->value).u.Boolean.boolean = (uint8_t) popOperand(thread->jvm_stack);
					break;
				case	REF_ARRAY:
					(var->value).u.ArrayReference.reference = (ARRAY *)(uintptr_t) popOperand(thread->jvm_stack);

					break;
				case	DOUBLE:
					(var->value).u.Double.low_bytes = popOperand(thread->jvm_stack);
					(var->value).u.Double.high_bytes = popOperand(thread->jvm_stack);
					break;
				case	LONG:
					(var->value).u.Long.low_bytes = popOperand(thread->jvm_stack);
					(var->value).u.Long.high_bytes = popOperand(thread->jvm_stack);
					break;
				default:
					puts("VerifyError: descritor de field inválido");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
			}
			break;
		case	getfield:
			objectref = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
			if(!objectref){
				puts("NullPointerException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			var = getInstanceVariable(cp_field_name, objectref);
			if(!var){
				puts("NoSuchFieldError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			if((var->field_reference)->modifiers & ACC_STATIC){
				puts("IncompatibleClassChangeError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			if((var->field_reference)->modifiers & ACC_PROTECTED){ // SE O FIELD É PROTECTED
				if(isSuperClass(field_class, method->CLASS_DATA, jvm)){// se a classe do field é superclasse da classe corrente.
					if((method->CLASS_DATA)->class_loader_reference != field_class->class_loader_reference){
						if(!(objectref->class_data_reference == method->CLASS_DATA) &&
						 !isSuperClass(method->CLASS_DATA, objectref->class_data_reference, jvm)){
						 	puts("IllegalAccessError");
						 	exitJVM(jvm);
						 	exit(EXIT_FAILURE);
						 }
					}
				}
			}
/*			printf("type = %u\n", (var->value).type);*/
			switch(field_descriptor[0]){
				case	BYTE:
					pushOperand((int32_t)(uintptr_t) (var->value).u.Byte.byte, thread->jvm_stack);
					break;
				case	CHAR:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.Char.char_, thread->jvm_stack);
					break;
				case	FLOAT:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.Float.float_, thread->jvm_stack);
					break;
				case	INT:
					pushOperand((int32_t)(uintptr_t) (var->value).u.Integer.integer, thread->jvm_stack);
					break;
				case	REF_INST:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.ArrayReference.reference, thread->jvm_stack);
					break;
				case	SHORT:
					pushOperand((int32_t)(uintptr_t) (var->value).u.Short.short_, thread->jvm_stack);
					break;
				case	BOOLEAN:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.Boolean.boolean, thread->jvm_stack);
					break;
				case	REF_ARRAY:
					pushOperand((uint32_t)(uintptr_t) (var->value).u.InstanceReference.reference, thread->jvm_stack);
					break;
				case	DOUBLE:
				case	LONG:;
					if((var->value).type == DOUBLE){
						pushOperand((uint32_t) (var->value).u.Double.high_bytes, thread->jvm_stack);
						pushOperand((uint32_t) (var->value).u.Double.low_bytes, thread->jvm_stack);
					}
					else{
						pushOperand((uint32_t) (var->value).u.Long.high_bytes, thread->jvm_stack);
						pushOperand((uint32_t) (var->value).u.Long.low_bytes, thread->jvm_stack);
					}
					break;
				default:
					puts("VerifyError: descritor de field inválido");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
			}
			break;
		case	putfield:;
			DATA_TYPES	value;
			switch(* field_descriptor){
				case	BYTE:
					value.u.Byte.byte = (int8_t) popOperand(thread->jvm_stack);
					break;
				case	CHAR:
					value.u.Char.char_ = (uint8_t) popOperand(thread->jvm_stack);
					break;
				case	FLOAT:
					value.u.Float.float_ = popOperand(thread->jvm_stack);
					break;
				case	INT:
					value.u.Integer.integer = (int32_t) popOperand(thread->jvm_stack);
					break;
				case	REF_INST:
					value.u.InstanceReference.reference = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
					break;
				case	SHORT:
					value.u.Short.short_ = (int16_t) popOperand(thread->jvm_stack);
					break;
				case	BOOLEAN:
					value.u.Boolean.boolean = (uint8_t) popOperand(thread->jvm_stack);
					break;
				case	REF_ARRAY:
					value.u.ArrayReference.reference = (ARRAY *)(uintptr_t) popOperand(thread->jvm_stack);

					break;
				case	DOUBLE:
					value.u.Double.low_bytes = popOperand(thread->jvm_stack);
					value.u.Double.high_bytes = popOperand(thread->jvm_stack);
					break;
				case	LONG:
					value.u.Long.low_bytes = popOperand(thread->jvm_stack);
					value.u.Long.high_bytes = popOperand(thread->jvm_stack);
					break;
				default:
					puts("VerifyError: descritor de field inválido");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
			}
			objectref = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
			if(!objectref){
				puts("NullPointerException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			var = getInstanceVariable(cp_field_name, objectref);
			if(!var){
				puts("NoSuchFieldError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			if((var->field_reference)->modifiers & ACC_PROTECTED){ // SE O FIELD É PROTECTED
				if(isSuperClass(field_class, method->CLASS_DATA, jvm)){// se a classe do field é superclasse da classe corrente.
					if((method->CLASS_DATA)->class_loader_reference != field_class->class_loader_reference){
						if(!(objectref->class_data_reference == method->CLASS_DATA) &&
						 !isSuperClass(method->CLASS_DATA, objectref->class_data_reference, jvm)){
						 	puts("IllegalAccessError");
						 	exitJVM(jvm);
						 	exit(EXIT_FAILURE);
						 }
					}
				}
			}
			var->value = value;
			break;
	}

	thread->program_counter += 3;
}

// invoke	0xB6 a 0xBA
//	invocação de métodos
void	invoke(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.invokevirtual*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.invokespecial*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.invokestatic*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.invokeinterface*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.invokedynamic*/

	uint8_t	indexbyte1 = *(thread->program_counter + 1);
	uint8_t	indexbyte2 = *(thread->program_counter + 2);
	uint16_t	index = (indexbyte1 << 8) | indexbyte2;

	

/*	https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-5.html#jvms-5.4.3.3*/
	// RESOLUÇÃO DO MÈTODO

	// nome da classe do método
	constant_pool_info	* cp = (thread->jvm_stack)->current_constant_pool;
	constant_pool_info * cp_method_ref = cp + index - 1; // falta verificar se o indice está nos limites da constant pool
	constant_pool_info * cp_class = cp + cp_method_ref->u.Ref.name_index - 1;
	constant_pool_info	* cp_class_name = cp + cp_class->u.Class.name_index - 1;
	char	* class_name = (char *) cp_class_name->u.Utf8.bytes;
	class_name[cp_class_name->u.Utf8.length] = '\0';

	// nome do método
	constant_pool_info	* cp_name_and_type = cp + cp_method_ref->u.Ref.name_and_type_index - 1;
	constant_pool_info * cp_method_name = cp + cp_name_and_type->u.NameAndType.name_index - 1;

	char	* method_name = (char *) cp_method_name->u.Utf8.bytes;
	method_name[cp_method_name->u.Utf8.length] = '\0';

	

	// descritor do método
	constant_pool_info	* cp_method_descriptor = cp + cp_name_and_type->u.NameAndType.descriptor_index - 1;
	char	* method_descriptor = (char *) cp_method_descriptor->u.Utf8.bytes;
	method_descriptor[cp_method_descriptor->u.Utf8.length] = '\0';

	// CONTROLE DE ACESSO
/*	https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-5.html#jvms-5.4.4*/
	uint8_t	* backupPC = thread->program_counter;
	CLASS_DATA	* method_class = getClass(cp_class_name, jvm);
	if(!method_class){// se a classe do método não foi carregada, mesmo package

		#ifdef	DEBUG
		puts("");
		#endif

		char	* string = malloc((strlen(class_name) + 7) * sizeof(CHAR)); // FREE OK
		strcpy(string, class_name);
		strcat(string, ".class");

		loadClass(string, &method_class, method->CLASS_DATA, jvm);
		free(string);
		linkClass(method_class, jvm);
		initializeClass(method_class, jvm, thread);
		thread->program_counter = backupPC;

		#ifdef	DEBUG_METODO
		puts("=======================");
		printf("Resume\t");
		PrintConstantUtf8((method->CLASS_DATA)->class_name, stdout);
		char	* current_method_name = (method->method_name)->u.Utf8.bytes;
		current_method_name[(method->method_name)->u.Utf8.length] = '\0';
		printf(".%s\n", current_method_name);
		puts("=======================");
		#endif
	}
	bool	is_print = false;
	bool	is_append = false;
	bool	is_toString = false;
	bool	is_initString = false;
	bool	is_equals = false;

	METHOD_DATA	* invoked_method = getMethod(method_name, method_descriptor, method_class, jvm);

	if(!invoked_method){
		puts("AbstractMethodError: método não encontrado");
		exitJVM(jvm);
		exit(EXIT_FAILURE);
	}

	if(!(invoked_method->modifiers & ACC_PUBLIC)){// SE O MÉTODO NÃO É PUBLICO
		if(invoked_method->modifiers & ACC_PROTECTED){ // SE O MÉTODO É PROTECTED
			if(method->CLASS_DATA != method_class){ // Se a classe do método invokado é diferente da classe método corrente
				if(!isSuperClass(method->CLASS_DATA, method_class, jvm)){
				// Se a classe do método invokado é não é subclasse da classe método corrente
					if((method->CLASS_DATA)->class_loader_reference != method_class->class_loader_reference){
						if(!(invoked_method->modifiers & ACC_PRIVATE)){
							puts("IllegalAccessError");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}
					}
				}
			}
			else if(1/*NOT restricao static*/){
				if(!(invoked_method->modifiers & ACC_PRIVATE)){
					puts("IllegalAccessError");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
				}
			}
			// restriçao static
		}
		else if(!invoked_method->modifiers){// SE O MÉTODO É DEFAULT
			if((method->CLASS_DATA)->class_loader_reference != (method_class)->class_loader_reference){
				if(!(invoked_method->modifiers & ACC_PRIVATE)){
					puts("IllegalAccessError");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	else{
		if(!strcmp(class_name, "java/io/PrintStream")){
			bool	ln = false;
			if(!strcmp(method_name, "println")){
				is_print = true;
				ln = true;
			}
			else if(!strcmp(method_name, "print")){
				is_print = true;
			}
			if(is_print){
				switch(method_descriptor[1]){
					case	BOOLEAN:;
						uint8_t	boolean_ = (uint8_t) popOperand(thread->jvm_stack);
						if(boolean_){
							printf("true");
						}
						else{
							printf("false");
						}
						break;
					case	BYTE:
						printf("%" PRId8, (int8_t) popOperand(thread->jvm_stack));
						break;
					case	CHAR:; // BUG PARA UNICODE CHAR
						uint16_t	char_ = popOperand(thread->jvm_stack);

						char * utf8_char = (char *) &char_;
						utf8_char[2] = '\0';
						printf("%c", utf8_char[0]);
						printf("%c", utf8_char[1]);
						break;
					case	SHORT:
						printf("%" PRId16, (int16_t) popOperand(thread->jvm_stack));
						break;
					case	INT:
						printf("%" PRId32, (int32_t) popOperand(thread->jvm_stack));
						break;
					case	FLOAT:;
						uint32_t float_bits = popOperand(thread->jvm_stack);
						bool	isValidFloat = true;
						switch(float_bits){
							case	0x7f800000:
								printf("+∞");
								isValidFloat = false;
								break;
							case	0xff800000:
								printf("-∞");
								isValidFloat = false;
								break;
							default:
								if((float_bits >= 0x7f800001 && float_bits <= 0x7fffffff) ||
									(float_bits >= 0xff800001 && float_bits <= 0xffffffff)){
									printf("NaN");
									isValidFloat = false;
								}

						}
						if(isValidFloat){
							int32_t s = ((float_bits >> 31) == 0) ? 1 : -1;
							int32_t e = ((float_bits >> 23) & 0xff);
							int32_t m = (e == 0) ?
								(float_bits & 0x7fffff) << 1 :
								(float_bits & 0x7fffff) | 0x800000;
							printf("%f", (float) s * m * pow(2,e - 150));
						}
						break;
					case	LONG:;
						uint32_t	long_low_bytes = popOperand(thread->jvm_stack);
						uint32_t	long_high_bytes = popOperand(thread->jvm_stack);
						int64_t	long_ = ((uint64_t) long_high_bytes << 32) | long_low_bytes;

						printf("%" PRId64, long_);
						break;
					case	DOUBLE:;
						uint32_t	double_low_bytes = popOperand(thread->jvm_stack);
						uint32_t	double_high_bytes = popOperand(thread->jvm_stack);
						uint64_t	double_bits = ((uint64_t) double_high_bytes << 32) | double_low_bytes;

						bool	isValidDouble = true;
						switch(double_bits){
							case	0x7ff0000000000000L:
								printf("+∞");
								isValidDouble = false;
								break;
							case	0xfff0000000000000L:
								printf("-∞");
								isValidDouble = false;
								break;
							default:
								if((double_bits >= 0x7ff0000000000001L && double_bits <= 0x7ffffffffffffL) ||
									(double_bits >= 0xfff0000000000001L && double_bits <= 0x7ffffffffffffL)){
									printf("NaN");
									isValidDouble = false;
								}

						}
						if(isValidDouble){
							int32_t s = ((double_bits >> 63) == 0) ? 1 : -1;
							int32_t e = ((double_bits >> 52) & 0x7ffL);
							int64_t m = (e == 0) ?
								(double_bits & 0xfffffffffffffL) << 1 :
								(double_bits & 0xfffffffffffffL) | 0x10000000000000L;
							printf("%f", (double) s * m * pow(2,e - 1075));
						}
						break;
					case	REF_INST:; // STRING
/*						printf("%s", (char *) popOperand(thread->jvm_stack));*/
						OBJECT	*	object_string = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
						printf("%s", (char *) (object_string->instance_variables)->value.u.InstanceReference.reference);
				}
				if(ln){
					puts("");
				}
			}
		}
		else if(!strcmp(class_name, "java/lang/StringBuffer")){
			if(!strcmp(method_name, "append")){
				is_append = true;
				char	* append_string;
				uint64_t	length;
				switch(method_descriptor[1]){
					case	BOOLEAN:;
						uint8_t	boolean = (uint8_t) popOperand(thread->jvm_stack);

						if(boolean){
							append_string = (char *) malloc(5 * sizeof(char));
							append_string[0] = '\0';
							strcpy(append_string, "true");
						}
						else{
							append_string = (char *) malloc(6 * sizeof(char));
							append_string[0] = '\0';
							strcpy(append_string, "false");
						}
						break;
					case	BYTE:;
						int8_t	byte = (int8_t) popOperand(thread->jvm_stack);

						if(byte < 0){
							length = -(byte/10) + 2;
							length++;
						}
						else{
							length = byte/10 + 2;
						}
						append_string = (char *) malloc(length * sizeof(char));
						sprintf(append_string, "%" PRId8, byte);
						break;
					case	CHAR:;
						uint16_t	char_ = (uint16_t) popOperand(thread->jvm_stack);

						append_string = (char *) malloc(2 * sizeof(char));
						sprintf(append_string, "%c", (char) char_);
						break;
					case	SHORT:;
						int16_t	short_ = (int16_t) popOperand(thread->jvm_stack);

						if(short_ < 0){
							length = -(short_/10) + 2;
							length++;
						}
						else{
							length = short_/10 + 2;
						}
						append_string = (char *) malloc(length * sizeof(char));
						sprintf(append_string, "%" PRId16, short_);
						break;
					case	INT:;
						int32_t	int_ = (int32_t) popOperand(thread->jvm_stack);

						if(int_ < 0){
							length = -(int_/10) + 2;
							length++;
						}
						else{
							length = int_/10 + 2;
						}

						append_string = (char *) malloc(length * sizeof(char));
						sprintf(append_string, "%" PRId32, int_);
						break;
					case	LONG:;
						uint32_t	long_low_bytes = popOperand(thread->jvm_stack);
						uint32_t	long_high_bytes = popOperand(thread->jvm_stack);
						int64_t	long_ = ((uint64_t) long_high_bytes << 32) | long_low_bytes;

						if(long_ < 0){
							length = -(long_/10) + 2;
							length++;
						}
						else{
							length = long_/10 + 2;
						}

						append_string = (char *) malloc(length * sizeof(char));
						sprintf(append_string, "%" PRId64, long_);
						break;
					case	FLOAT:;
						uint32_t float_bits = popOperand(thread->jvm_stack);
						bool	isValidFloat = true;
						switch(float_bits){
							case	0x7f800000:
								append_string = (char *) malloc(3 * sizeof(char));
								sprintf(append_string, "+∞");
								isValidFloat = false;
								break;
							case	0xff800000:
								append_string = (char *) malloc(3 * sizeof(char));
								sprintf(append_string, "-∞");
								isValidFloat = false;
								break;
							default:
								if((float_bits >= 0x7f800001 && float_bits <= 0x7fffffff) ||
									(float_bits >= 0xff800001 && float_bits <= 0xffffffff)){
									append_string = (char *) malloc(4 * sizeof(char));
									sprintf(append_string, "NaN");
									isValidFloat = false;
								}

						}
						if(isValidFloat){
							int32_t s = ((float_bits >> 31) == 0) ? 1 : -1;
							int32_t e = ((float_bits >> 23) & 0xff);
							int32_t m = (e == 0) ?
								(float_bits & 0x7fffff) << 1 :
								(float_bits & 0x7fffff) | 0x800000;
							append_string = (char *) malloc(13 * sizeof(char));
							sprintf(append_string, "%E", (float) s * m * pow(2,e - 150));
						}
						break;
					case	DOUBLE:;
						uint32_t	double_low_bytes = popOperand(thread->jvm_stack);
						uint32_t	double_high_bytes = popOperand(thread->jvm_stack);
						uint64_t	double_bits = ((uint64_t) double_high_bytes << 32) | double_low_bytes;

						bool	isValidDouble = true;
						switch(double_bits){
							case	0x7ff0000000000000L:
								append_string = (char *) malloc(3 * sizeof(char));
								sprintf(append_string, "+∞");
								isValidDouble = false;
								break;
							case	0xfff0000000000000L:
								append_string = (char *) malloc(3 * sizeof(char));
								sprintf(append_string, "-∞");
								isValidDouble = false;
								break;
							default:
								if((double_bits >= 0x7ff0000000000001L && double_bits <= 0x7ffffffffffffL) ||
									(double_bits >= 0xfff0000000000001L && double_bits <= 0x7ffffffffffffL)){
									append_string = (char *) malloc(4 * sizeof(char));
									sprintf(append_string, "NaN");
									isValidDouble = false;
								}

						}
						if(isValidDouble){
							int32_t s = ((double_bits >> 63) == 0) ? 1 : -1;
							int32_t e = ((double_bits >> 52) & 0x7ffL);
							int64_t m = (e == 0) ?
								(double_bits & 0xfffffffffffffL) << 1 :
								(double_bits & 0xfffffffffffffL) | 0x10000000000000L;
							append_string = (char *) malloc(14 * sizeof(char));
							sprintf(append_string, "%E", (double) s * m * pow(2,e - 1075));
						}
						break;
					case	REF_INST:
						append_string = (char *) (((OBJECT *)(uintptr_t) popOperand(thread->jvm_stack))->instance_variables)->value.u.InstanceReference.reference;
						break;
				}
				OBJECT * string_buffer = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
				OBJECT * aux = (OBJECT *) malloc((strlen(append_string) +
				strlen((char *) (string_buffer->instance_variables)->value.u.InstanceReference.reference) + 1) * sizeof(char));
				
				strcpy((char *) aux, (char *) (string_buffer->instance_variables)->value.u.InstanceReference.reference);
/*				(string_buffer->instance_variables)->value.u.InstanceReference.reference =*/
/*				(OBJECT *) realloc((string_buffer->instance_variables)->value.u.InstanceReference.reference,*/
/*				strlen(append_string) +*/
/*				strlen((char *) (string_buffer->instance_variables)->value.u.InstanceReference.reference) + 1);*/
				
				strcat((char *) aux, append_string);
				
				free((string_buffer->instance_variables)->value.u.InstanceReference.reference);
				(string_buffer->instance_variables)->value.u.InstanceReference.reference = aux;
				
/*				strcat((char *) (string_buffer->instance_variables)->value.u.InstanceReference.reference, append_string);*/
				pushOperand((uint32_t)(uintptr_t) string_buffer, thread->jvm_stack);
				
				if(method_descriptor[1] != REF_INST){
					free(append_string);
				}
				
				#ifdef	DEBUG
				printf("string_buffer:%s", (char *) string_buffer);
				#endif
			}
			else if(!strcmp(method_name, "toString")){
				is_toString = true;
			}
		}
		else if(!strcmp(class_name, "java/lang/String")){
			if(!strcmp(method_name, "<init>")){
				is_initString = true;
				OBJECT	* object_string = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
				OBJECT	* objectref = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
				(objectref->instance_variables)->value.u.InstanceReference.reference =
				(object_string->instance_variables)->value.u.InstanceReference.reference;
			}
			else if(!strcmp(method_name, "equals")){
				is_equals = true;
				OBJECT	* object_string = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
				OBJECT	* objectref = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
				if(!strcmp((char *) (objectref->instance_variables)->value.u.InstanceReference.reference,
					(char *) (object_string->instance_variables)->value.u.InstanceReference.reference)){
					pushOperand(1, thread->jvm_stack);
				}
				else{
					pushOperand(0, thread->jvm_stack);
				}
				
				
			}
		}
	}


	uint16_t	nargs;
	uint32_t	* args;
	if(!is_print && !is_append && !is_toString && !is_initString && !is_equals){
		uint16_t	i = 1;
		nargs = 0;
		while(method_descriptor[i] != ')'){
			switch(method_descriptor[i]){
				case	REF_INST:
					while(method_descriptor[i] != ';'){
						i++;
					}
				case	BOOLEAN:
				case	BYTE:
				case	CHAR:
				case	FLOAT:
				case	INT:
				case	SHORT:
					nargs++;
					break;
				case	LONG:
				case	DOUBLE:
					nargs += 2;
					break;
				case	REF_ARRAY:
					while(method_descriptor[i] == '['){
						i++;
					}
					switch(method_descriptor[i]){
						case	REF_INST:
							while(method_descriptor[i] != ';'){
								i++;
							}
						case	BOOLEAN:
						case	BYTE:
						case	CHAR:
						case	FLOAT:
						case	INT:
						case	SHORT:
						case	LONG:
						case	DOUBLE:
							nargs++;
							break;
					}
					break;
			}
			i++;
		}

		// desempilha operandos e coloca no vetor de variaveis locais;
		uint16_t	args_count = nargs;
		if(args_count){
			args = (uint32_t *) malloc(args_count * sizeof(uint32_t));
		}
		else{
			args = NULL;
		}
		
		nargs = 0;
		i = 1;

		while(method_descriptor[i] != ')'){
			switch(method_descriptor[i]){
				case	REF_INST:
					while(method_descriptor[i] != ';'){
						i++;
					}
				case	BOOLEAN:
				case	BYTE:
				case	CHAR:
				case	FLOAT:
				case	INT:
				case	SHORT:
					
					args[args_count - nargs - 1] = popOperand(thread->jvm_stack);
					nargs++;
					break;
				case	LONG:
				case	DOUBLE:
					
					args[args_count - nargs - 1] = popOperand(thread->jvm_stack);
					nargs++;

					
					args[args_count - nargs - 1] = popOperand(thread->jvm_stack);
					nargs++;
					break;
				case	REF_ARRAY:
					while(method_descriptor[i] == '['){
						i++;
					}
					switch(method_descriptor[i]){
						case	REF_INST:
							while(method_descriptor[i] != ';'){
								i++;
							}
						case	BOOLEAN:
						case	BYTE:
						case	CHAR:
						case	FLOAT:
						case	INT:
						case	SHORT:
						case	LONG:
						case	DOUBLE:
						
							args[args_count - nargs - 1] = popOperand(thread->jvm_stack);
							nargs++;
							break;
					}
					break;
			}
			i++;
		}
	}
	OBJECT	* objectref;
	CLASS_DATA	* super_class;
	bool		findMethod;
	switch(* thread->program_counter){// PARA TESTAR O HELLOWORLD
		case	invokevirtual:
			if(is_print || is_append || is_toString || is_initString || is_equals){ // NATIVE METHOD
				break;
			}
			if((!strcmp(method_name, "<init>")) || (!strcmp(method_name, "<clinit>"))){
				puts("InvokeVirtualInitError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			objectref = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
			if(!objectref){
				puts("NullPointerException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			if(invoked_method->modifiers & ACC_STATIC){
				puts("IncompatibleClassChangeError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			CLASS_DATA	* class_objectref = objectref->class_data_reference;
			if((invoked_method->modifiers & ACC_PROTECTED)){
				CLASS_DATA	* super_class = getSuperClass((method->CLASS_DATA)->classfile, jvm);
				bool	isSuperClass = false;
				while(super_class && !isSuperClass){
					if(method_class == super_class){
						isSuperClass = true;
					}
					else{
						super_class =  getSuperClass(super_class->classfile, jvm);
					}
				}
				if(isSuperClass && ((invoked_method->CLASS_DATA)->class_loader_reference !=
							 (method->CLASS_DATA)->class_loader_reference )){
					if(class_objectref != method->CLASS_DATA){
						isSuperClass = false;
						super_class = getSuperClass(class_objectref->classfile, jvm);
						while(super_class && !isSuperClass){
							if(method->CLASS_DATA == super_class){
								isSuperClass = true;
							}
							else{
								super_class = getSuperClass(super_class->classfile, jvm);
							}
						}
						if(!isSuperClass){
							puts("IllegalAccessError: acesso indevido à método protegido");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}
					}
				}
			}
			findMethod	 = false;
			super_class = class_objectref;
			while(super_class && !findMethod){
				if((invoked_method = getMethod(method_name, method_descriptor, super_class, jvm))){
					if(!(invoked_method->modifiers & ACC_STATIC)){
						backupPC = thread->program_counter;
						if(invoked_method->modifiers & ACC_NATIVE){
							puts("UnsatisfiedLinkError");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}

						if(invoked_method->modifiers & ACC_ABSTRACT){
							puts("AbstractMethodError: método abstrato");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}
						executeMethod(method_name, method_descriptor, invoked_method->CLASS_DATA,
								jvm, thread, objectref, nargs, args);
						thread->program_counter = backupPC;
						findMethod = true;
					}
					else{
						super_class = getSuperClass(super_class->classfile, jvm);
					}
				}
				else{
					super_class = getSuperClass(super_class->classfile, jvm);
				}
			}
			if(!findMethod){
			}
				puts("AbstractMethodError:");
				exitJVM(jvm);
				exit(EXIT_FAILURE);

			break;
		case	invokespecial:;
			if(is_initString){
				break;
			}
			objectref= (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
			if(!objectref){
				puts("NullPointerException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			if(invoked_method->modifiers & ACC_STATIC){
				puts("IncompatibleClassChangeError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			if((invoked_method->modifiers & ACC_PROTECTED)){
				CLASS_DATA	* super_class = getSuperClass((method->CLASS_DATA)->classfile, jvm);
				bool	isSuperClass = false;
				while(super_class && !isSuperClass){
					if(method_class == super_class){
						isSuperClass = true;
					}
					else{
						super_class =  getSuperClass(super_class->classfile, jvm);
					}
				}
				if(isSuperClass && ((invoked_method->CLASS_DATA)->class_loader_reference !=
							 (method->CLASS_DATA)->class_loader_reference )){
					CLASS_DATA	* class_objectref = objectref->class_data_reference;
					if(class_objectref != method->CLASS_DATA){
						isSuperClass = false;
						super_class = getSuperClass(class_objectref->classfile, jvm);
						while(super_class && !isSuperClass){
							if(method->CLASS_DATA == super_class){
								isSuperClass = true;
							}
							else{
								super_class = getSuperClass(super_class->classfile, jvm);
							}
						}
						if(!isSuperClass){
							puts("IllegalAccessError: acesso indevido à método protegido");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}
					}
				}
			}
			CLASS_DATA	* super_class = getSuperClass((method->CLASS_DATA)->classfile, jvm);
				bool	isSuperClass = false;
				while(super_class && !isSuperClass){
					if(method_class == super_class){
						isSuperClass = true;
					}
					else{
						super_class =  getSuperClass(super_class->classfile, jvm);
					}
				}
			// se a classe corrente é super;
			// a classe do método invokado é superclasse da classe corrente;
			// e o metodo nao é <init>

			if(((method->CLASS_DATA)->modifiers & ACC_SUPER) && (isSuperClass) && strcmp(method_name, "<init>")){
				bool	findMethod = false;
				super_class = getSuperClass((method->CLASS_DATA)->classfile, jvm);
				while(super_class && !findMethod){
					if((invoked_method = getMethod(method_name, method_descriptor, super_class, jvm))){
						if(!(invoked_method->modifiers & ACC_STATIC)){
							if(invoked_method->modifiers & ACC_ABSTRACT){
								puts("AbstractMethodError: método abstrato");
								exitJVM(jvm);
								exit(EXIT_FAILURE);
							}
							if(invoked_method->modifiers & ACC_NATIVE){
								puts("UnsatisfiedLinkError");
								exitJVM(jvm);
								exit(EXIT_FAILURE);
							}

							backupPC = thread->program_counter;
							executeMethod(method_name, method_descriptor, invoked_method->CLASS_DATA,
									jvm, thread, objectref, nargs, args);
							thread->program_counter = backupPC;
							findMethod = true;
						}
						else{
							super_class = getSuperClass(super_class->classfile, jvm);
						}
					}
					else{
						super_class = getSuperClass(super_class->classfile, jvm);
					}
				}
				if(!findMethod){
					puts("AbstractMethodError:");
					exitJVM(jvm);
					exit(EXIT_FAILURE);
				}

			}
			else{
				backupPC = thread->program_counter;
				executeMethod(method_name, method_descriptor, method_class, jvm, thread, objectref, nargs, args);
				thread->program_counter = backupPC;
			}


			break;
		case	invokestatic:
			if((!strcmp(method_name, "<init>")) || (!strcmp(method_name, "<clinit>"))){
				puts("InvokeStaticInitError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			if(!(invoked_method->modifiers & ACC_STATIC) || (invoked_method->modifiers & ACC_ABSTRACT)){
				puts("IllegalAccessError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			if(invoked_method->modifiers & ACC_NATIVE){
				puts("UnsatisfiedLinkError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			backupPC = thread->program_counter;
			executeMethod(method_name, method_descriptor, method_class, jvm, thread, NULL, nargs, args);
			thread->program_counter = backupPC;
			break;
		case	invokeinterface:
			if(cp_method_ref->tag != CONSTANT_InterfaceMethodref){
				puts("VerifyError: invokeinterface invalid method");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			objectref = (OBJECT *)(uintptr_t) popOperand(thread->jvm_stack);
			if(!objectref){
				puts("NullPointerException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			if((!strcmp(method_name, "<init>")) || (!strcmp(method_name, "<clinit>"))){
				puts("InvokeInterfaceInitError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			uint8_t	count = * (thread->program_counter + 3);
			if(!count){
				puts("InvokeInterfaceError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			#ifdef	DEBUG
			printf("\tcount	%" PRIuint64_t, count);
			#endif

			uint32_t	fourth_operand = * (thread->program_counter + 4);
			if(fourth_operand){
				printf("\n%" PRIu32, fourth_operand);
				puts("InvokeInterfaceError");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			class_objectref = objectref->class_data_reference;

			findMethod = false;
			super_class = class_objectref;
			while(super_class && !findMethod){
				if((invoked_method = getMethod(method_name, method_descriptor, super_class, jvm))){
					if(!(invoked_method->modifiers & ACC_STATIC)){
						backupPC = thread->program_counter;
						if(invoked_method->modifiers & ACC_ABSTRACT){
							puts("AbstractMethodError: método abstrato");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}
						if(invoked_method->modifiers & ACC_NATIVE){
							puts("UnsatisfiedLinkError");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}

						if(!invoked_method->modifiers & ACC_PUBLIC){
							puts(" IllegalAccessError");
							exitJVM(jvm);
							exit(EXIT_FAILURE);
						}

						executeMethod(method_name, method_descriptor, invoked_method->CLASS_DATA,
								jvm, thread, objectref, nargs, args);
						thread->program_counter = backupPC;
						findMethod = true;
					}
					else{
						super_class = getSuperClass(super_class->classfile, jvm);
					}
				}
				else{
					super_class = getSuperClass(super_class->classfile, jvm);
				}
			}
			if(!findMethod){
				puts("AbstractMethodError: método não encontrado");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			thread->program_counter += 2;
			break;
		case	invokedynamic:
			thread->program_counter += 2;
			break;
	}
	thread->program_counter += 3;

}

// handleObject	0xBB a 0xBE; 0xC5
//	criação e manipulação de objetos e arrays
void	handleObject(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.new*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.newarray*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.anewarray*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.arraylength*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.multianewarray*/

	int32_t	count;
	uint8_t	indexbyte1;
	uint8_t	indexbyte2;
	uint16_t	index;
	constant_pool_info	* cp_class;
	constant_pool_info	* cp;
	constant_pool_info	* cp_class_name;
	char	* class_name;
	ARRAY *	new_array;
	uint8_t	* backupPC;
	ARRAY	* arrayref;

	switch(* thread->program_counter){// PARA TESTAR O HELLOWORLD
		case	new:
			indexbyte1 = * (thread->program_counter + 1);
			indexbyte2 = * (thread->program_counter + 2);
			index = (indexbyte1 << 8) | indexbyte2;

			cp =(thread->jvm_stack)->current_constant_pool;
			cp_class = cp + index - 1;

			if(cp_class->tag != CONSTANT_Class){
				puts("InstantiationError: Referência inválida para classe do objeto");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			cp_class_name = cp + cp_class->u.Class.name_index - 1;

			class_name = (char *) cp_class_name->u.Utf8.bytes;
			class_name[cp_class_name->u.Utf8.length] = '\0';

			OBJECT	*	newObject;
			if(!strcmp(class_name, "java/lang/StringBuffer") || !strcmp(class_name, "java/lang/String")){
				char * string = (char *) malloc(sizeof(char));
				string[0] = '\0';
				newObject = (OBJECT *)  malloc(sizeof(OBJECT));
				newObject->next = (jvm->heap_jvm)->objects;
				(jvm->heap_jvm)->objects = newObject;
				newObject->instance_variables = (VARIABLE *) malloc(sizeof(VARIABLE));
				(newObject->instance_variables)->value.u.InstanceReference.reference = (OBJECT *) string;
				
				
				pushOperand((uint32_t)(uintptr_t) newObject, thread->jvm_stack);
				thread->program_counter += 3;
				return;
			}

			// CONTROLE DE ACESSO
			backupPC = thread->program_counter;
			CLASS_DATA	* object_class = getClass(cp_class_name, jvm);
			if(!object_class){// se a classe do objeto não foi carregada

				char	* string = malloc((strlen(class_name) + 7) * sizeof(CHAR));
				strcpy(string, class_name);
				strcat(string, ".class");
				loadClass(string, &object_class, method->CLASS_DATA, jvm);
				free(string);
				linkClass(object_class, jvm);
				initializeClass(object_class, jvm, thread);

				thread->program_counter = backupPC;
			}
			else{
				if(object_class != method->CLASS_DATA){// Se o objeto não for da mesma classe do método
					// verifica se a classe do objeto é acessível pelo método corrente
					if(!(object_class->modifiers & ACC_PUBLIC) & // se a classe não é public e não é do mesmo package
					(object_class->class_loader_reference != (method->CLASS_DATA)->class_loader_reference)){
						puts("IllegalAccessError: acesso indevido à classe ou interface.");
						exitJVM(jvm);
						exit(EXIT_FAILURE);
					}
				}
			}
			if((object_class->modifiers == ACC_INTERFACE) || (object_class->modifiers == ACC_ABSTRACT)){
				puts("InstantiationError: Criação de objeto de interface ou class abstrata");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			//newObject = (OBJECT *) malloc(sizeof(OBJECT));

			newObject = (OBJECT *) malloc(sizeof(OBJECT));
			newObject->class_data_reference = object_class;
			newObject->next = (jvm->heap_jvm)->objects;
			(jvm->heap_jvm)->objects = newObject;

			newObject->instance_variables = NULL;
			CLASS_DATA	* super_class = object_class;
			// CRIA INSTANCE_VARIABLES
			while(super_class){
				if((super_class->classfile)->fields_count){
					for(uint16_t i = 0; i < (super_class->classfile)->fields_count; i++){
						VARIABLE	* var = (VARIABLE *) malloc(sizeof(VARIABLE));
						var->field_reference = super_class->field_data + i;
						(super_class->field_data + i)->var = var;
	
						uint16_t	descriptor_index = ((super_class->field_data + i)->info)->descriptor_index;
						(var->value).type = (super_class->runtime_constant_pool + descriptor_index - 1)->u.Utf8.bytes[0];
						switch((var->value).type){
							case	BOOLEAN:
								(var->value).u.Boolean.boolean = 0;
								break;
							case	BYTE:
								(var->value).u.Byte.byte = 0;
								break;
							case	CHAR:
								(var->value).u.Char.char_ = 0;
								break;
							case	DOUBLE:
								(var->value).u.Double.high_bytes = 0;
								(var->value).u.Double.low_bytes = 0;
								break;
							case	FLOAT:
								(var->value).u.Float.float_ = 0;
								break;
							case	INT:
								(var->value).u.Integer.integer = 0;
								break;
							case	LONG:
								(var->value).u.Long.high_bytes = 0;
								(var->value).u.Long.low_bytes = 0;
								break;
							case	REF_INST:;
								(var->value).u.InstanceReference.reference = NULL;
								break;
							case	SHORT:
								(var->value).u.Short.short_ = 0;
								break;
							case	REF_ARRAY:
								(var->value).u.ArrayReference.reference = NULL;
								break;
							default:
								puts("VerifyError: Unknown type");
								exitJVM(jvm);
								exit(EXIT_FAILURE);
						}
						uint16_t	access_flags = (super_class->field_data + i)->modifiers;
/*			if(!(acc	ess_flags & ACC_FINAL)){*/
						if(!(access_flags & ACC_STATIC)){
							var->next = newObject->instance_variables;
								newObject->instance_variables = var;
						}
						else{
							free(var);
						}
/*			}*/
					}
				}
				super_class = getSuperClass(super_class->classfile, jvm);
			}


			// coloca novo objeto no heap_jvm
			newObject->next = (jvm->heap_jvm)->objects;
			(jvm->heap_jvm)->objects = newObject;

			// coloca referencia do objeto na pilha
			pushOperand((uint32_t)(uintptr_t) newObject, thread->jvm_stack);
			thread->program_counter += 3;
			break;
		case	anewarray:
			count = popOperand(thread->jvm_stack);
			if(count < 0){
				puts("NegativeArraySizeException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			indexbyte1 = *(thread->program_counter + 1);
			indexbyte2 = *(thread->program_counter + 2);
			index = (indexbyte1 << 8) | indexbyte2;

		
			cp = (thread->jvm_stack)->current_constant_pool;
			cp_class = cp + index - 1;

			if(cp_class->tag !=CONSTANT_Class){
				puts("VerifyError: invalid anewarray index ");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			cp_class_name = cp + cp_class->u.Class.name_index - 1;

			class_name = (char *) cp_class_name->u.Utf8.bytes;
			class_name[cp_class_name->u.Utf8.length] = '\0';

			// CONTROLE DE ACESSO
			backupPC = thread->program_counter;
			CLASS_DATA	* array_class = getClass(cp_class_name, jvm);
			if(!array_class){// se a classe do array não foi carregada

				char	* string = malloc((strlen(class_name) + 7) * sizeof(CHAR));
				strcpy(string, class_name);
				strcat(string, ".class");

				loadClass(string, &array_class, method->CLASS_DATA, jvm);
				free(string);
				linkClass(array_class, jvm);
				initializeClass(array_class, jvm, thread);

				thread->program_counter = backupPC;
			}
			else{
				if(array_class != method->CLASS_DATA){// Se o array não for da mesma classe do método
					// verifica se a classe do arrayé acessível pelo método corrente
					if(!(array_class->modifiers & ACC_PUBLIC) & // se a classe não é public e não é do mesmo package
					(array_class->class_loader_reference != (method->CLASS_DATA)->class_loader_reference)){
						puts("IllegalAccessError: acesso indevido à classe ou interface.");
						exitJVM(jvm);
						exit(EXIT_FAILURE);
					}
				}
			}
			new_array = (ARRAY *) malloc(sizeof(ARRAY));
			new_array->class_data_reference = array_class;
			new_array->count = count;
			new_array->entry = (DATA_TYPES *) malloc(count * sizeof(DATA_TYPES));

			if(!strcmp(class_name, "java/lang/String")){
				for(uint32_t i = 0; i < count; i++){
					char * string = (char *) malloc(sizeof(char));
					string[0] = '\0';
					newObject = (OBJECT *)  malloc(sizeof(OBJECT));
					newObject->instance_variables = (VARIABLE *) malloc(sizeof(VARIABLE));
					(newObject->instance_variables)->value.u.InstanceReference.reference = (OBJECT *) string;
					(new_array->entry + i)->type = REF_INST;
					(new_array->entry + i)->u.InstanceReference.reference = newObject;
				}

			}
			else{
				for(uint32_t i = 0; i < count; i++){
					(new_array->entry + i)->type = REF_INST;
					(new_array->entry + i)->u.InstanceReference.reference = NULL;
				}
			}


			new_array->next = (jvm->heap_jvm)->arrays;
			(jvm->heap_jvm)->arrays = new_array;

			pushOperand((uint32_t)(uintptr_t) new_array, thread->jvm_stack);

			thread->program_counter += 3;
			break;

		case	newarray:;
			count = popOperand(thread->jvm_stack);

			if(count < 0){
				puts("NegativeArraySizeException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			new_array = (ARRAY *) malloc(sizeof(ARRAY));
			new_array->atype = * (thread->program_counter + 1);
			new_array->count = count;
			new_array->entry = (DATA_TYPES *) malloc(count * sizeof(DATA_TYPES));
			new_array->next = (jvm->heap_jvm)->arrays;
			(jvm->heap_jvm)->arrays = new_array;

			pushOperand((uint32_t)(uintptr_t) new_array, thread->jvm_stack);

			thread->program_counter += 2;
			break;
		case	arraylength:
			arrayref = (ARRAY *) (uintptr_t) popOperand(thread->jvm_stack);
			if(!arrayref){
				puts("NullPointerException");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}
			pushOperand((int32_t) arrayref->count, thread->jvm_stack);
			thread->program_counter++;
			break;
		case	multianewarray:
			indexbyte1 = *(thread->program_counter + 1);
			indexbyte2 = *(thread->program_counter + 2);
			index = (indexbyte1 << 8) | indexbyte2;

			uint8_t	dimensions = *(thread->program_counter + 3);
		
			cp = (thread->jvm_stack)->current_constant_pool;
			cp_class = cp + index - 1;

			if(cp_class->tag !=CONSTANT_Class){
				puts("VerifyError: invalid multianewarray index ");
				exitJVM(jvm);
				exit(EXIT_FAILURE);
			}

			cp_class_name = cp + cp_class->u.Class.name_index - 1;

			class_name = (char *) cp_class_name->u.Utf8.bytes;
			class_name[cp_class_name->u.Utf8.length] = '\0';


			// CONTROLE DE ACESSO
			backupPC = thread->program_counter;
			array_class = getClass(cp_class_name, jvm);
			if(!array_class){// se a classe do array não foi carregada
				if(class_name[0] == '['){
					array_class = (CLASS_DATA *) malloc(sizeof(CLASS_DATA));
					array_class->class_name = cp_class_name;
					array_class->classfile = NULL;
					array_class->modifiers = ACC_PUBLIC;
					array_class->class_loader_reference = method->CLASS_DATA;
					array_class->runtime_constant_pool = NULL;
					array_class->field_data = NULL;
					array_class->method_data = NULL;
					array_class->class_variables = NULL;
					array_class->next = jvm->class_data_jvm;
					jvm->class_data_jvm = array_class;
				}
				else{
					char	* string = malloc((strlen(class_name) + 7) * sizeof(CHAR));
					strcpy(string, class_name);
					strcat(string, ".class");

					loadClass(string, &array_class, method->CLASS_DATA, jvm);
					free(string);
					linkClass(array_class, jvm);
					initializeClass(array_class, jvm, thread);

					thread->program_counter = backupPC;
				}
			}
			else{
				if(array_class != method->CLASS_DATA){// Se o array não for da mesma classe do método
					// verifica se a classe do arrayé acessível pelo método corrente
					if(!(array_class->modifiers & ACC_PUBLIC) & // se a classe não é public e não é do mesmo package
					(array_class->class_loader_reference != (method->CLASS_DATA)->class_loader_reference)){
						puts("IllegalAccessError: acesso indevido à classe ou interface.");
						exitJVM(jvm);
						exit(EXIT_FAILURE);
					}
				}
			}

			int32_t	* count_values = (int32_t *) malloc(dimensions * sizeof(int32_t));
			for(uint8_t i = 0; i < dimensions; i++){
				count_values[i] = popOperand(thread->jvm_stack);
			}

			arrayref = (ARRAY *) malloc(sizeof(ARRAY));
			arrayref->next = (jvm->heap_jvm)->arrays;
			(jvm->heap_jvm)->arrays = arrayref;
			
			arrayref->class_data_reference = array_class;
			createMultiArray(arrayref, count_values, 0, dimensions, jvm);

			pushOperand((uint32_t)(uintptr_t) arrayref, thread->jvm_stack);

			free(count_values);
			thread->program_counter += 4;
			break;
	}
}

// athrow	0xBF
//	lançamento de exceções
void	athrow_(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.athrow*/
}

// properties	0xC0 e 0xC1
//	checa propriedades de objetos
void	properties(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.checkcast*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.instanceof*/
	switch(* thread->program_counter){
		case	checkcast:;
/*			uint8_t	indexbyte1 = * (thread->program_counter + 1);*/
/*			uint8_t	indexbyte2 = * (thread->program_counter + 2);*/
/*			uint16_t	index = (indexbyte1 << 8) | indexbyte2;*/
/*			*/
/*			constant_pool_info	* cp = (thread->jvm_stack)->current_constant_pool;*/
/*			constant_pool_info	* cp_class = cp + index - 1;*/
/*			if(cp_class->tag != CONSTANT_Class){*/
/*				puts("VerifyError: invalid checkcast index");*/
/*				exit(EXIT_FAILURE);*/
/*			}*/
/*			OBJECT	* objectref = (OBJECT *) popOperand(thread->jvm_stack);*/
/*			if(!objectref){*/
/*				pushOperand((uint32_t) objectref, thread->jvm_stack);*/
/*			}*/
/*			else{*/
/*				*/
/*			}*/
/*				*/
			break;
		case	instanceof:
			break;
	}
	thread->program_counter += 3;
}

// monitor	0xC2 e 0xC3
//	implementa monitores para objetos (multithreading)
void	monitor(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.monitorenter*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.monitorexit*/
}

// wide		0xC4
//	modifica tamanho de argumentos de certas instruções aritmeticas
void	wide_(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.wide*/
	isWide = 1;

	thread->program_counter++;
}

void	createMultiArray(ARRAY * arrayref, int32_t * count_values, uint8_t i, uint8_t dimensions, JVM * jvm){
	if(i < dimensions){
		if(count_values[i] != 0){
			arrayref->count = count_values[i];
			arrayref->entry = (DATA_TYPES *) malloc(arrayref->count * sizeof(DATA_TYPES));
			i++;
			for(int32_t j = 0; j < arrayref->count; j++){
				arrayref->entry[j].u.ArrayReference.reference = (ARRAY *) malloc(sizeof(ARRAY));
				(arrayref->entry[j].u.ArrayReference.reference)->next = (jvm->heap_jvm)->arrays;
				(jvm->heap_jvm)->arrays = arrayref->entry[j].u.ArrayReference.reference;
				(arrayref->entry[j].u.ArrayReference.reference)->class_data_reference = arrayref->class_data_reference;
				createMultiArray(arrayref->entry[j].u.ArrayReference.reference, count_values, i, dimensions, jvm);
			}
		}
	}
}


// ifNull	0xC6 e 0xC7
//	verifica se uma referencia é ou não NULL, e realiza jump
void	ifNull(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifnull*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.ifnonnull*/
    uint8_t	branchbyte1 = * (thread->program_counter + 1);
	uint8_t	branchbyte2 = * (thread->program_counter + 2);
	int16_t	branch = (branchbyte1 << 8) | branchbyte2;

    int32_t value = (int32_t) popOperand(thread->jvm_stack);
	switch(* thread->program_counter){
		case	ifnull:
            if(value == 0){
                thread->program_counter += branch;
            }else{
                thread->program_counter += 3;
            }
			break;
        case	ifnonnull:
            if(value != 0){
                thread->program_counter += branch;
            }else{
                thread->program_counter += 3;
            }
			break;
	}
}

// widejump	0xC8 a 0xC9
//	jumps incondicionais de maior tamanho de offset
void	widejump(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.goto_w*/
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.5.jsr_w*/
    uint8_t	branchbyte1 = * (thread->program_counter + 1);
	uint8_t	branchbyte2 = * (thread->program_counter + 2);
	uint8_t	branchbyte3 = * (thread->program_counter + 3);
	uint8_t	branchbyte4 = * (thread->program_counter + 4);

	int32_t	branch = (int32_t)(((branchbyte1 & 0xFF)<<24) | ((branchbyte2 & 0xFF)<<16) | ((branchbyte3 & 0xFF)<<8) | (branchbyte4 & 0xFF));

	switch(* thread->program_counter){
		case	goto_w:
            thread->program_counter += branch;
			break;
        case	jsr_w:
            
			pushOperand((uint32_t)(uintptr_t)(thread->program_counter += 5), thread->jvm_stack);
            thread->program_counter += branch;
			break;
	}
}

// breakpoint	0xCA
//	(opcode reservado) utilizado por debuggers para implementar breakpoints
void	breakpoint_(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.2*/
}

// nonDefined	0xCB a 0xFD
//	faixa de Opcodes não utilizada pela Oracle
void	nonDefined(METHOD_DATA * method, THREAD * thread, JVM * jvm){
	puts("Unknown jvm instruction.");
	exitJVM(jvm);
	exit(EXIT_FAILURE);
}

// impdep	0xFE e 0xFF
//	(opcodes reservados) instruções que provêem backdoors para implementações específicas
void	impdep(METHOD_DATA * method, THREAD * thread, JVM * jvm){
/*https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html#jvms-6.2*/
}


// ARRAY DE PONTEIROS PRA FUNÇÃO
INSTRUCTION	func[] = {
		// 0x00		nop
		nop_,

		// 0x01 a 0x0F	Tconst
		Tconst, Tconst, Tconst, Tconst, Tconst, Tconst, Tconst,Tconst,
		Tconst,	Tconst, Tconst, Tconst, Tconst, Tconst, Tconst,

		// 0x10 e 0x11	Tipush
		Tipush, Tipush,

		// 0x12 a 0x14	ldc_
		ldc_, ldc_, ldc_,

		// 0x15 a 0x2D	Tload
		Tload, Tload, Tload, Tload, Tload, Tload, Tload, Tload, Tload, Tload,
		Tload, Tload, Tload, Tload, Tload, Tload, Tload, Tload, Tload, Tload,
		Tload, Tload, Tload, Tload, Tload,

		// 0x2E a 0x35	Taload
		Taload, Taload, Taload, Taload, Taload, Taload, Taload, Taload,

		// 0x36 a 0x4E	Tstore
		Tstore, Tstore, Tstore, Tstore, Tstore, Tstore,	Tstore, Tstore, Tstore, Tstore,
		Tstore, Tstore, Tstore, Tstore, Tstore, Tstore,	Tstore, Tstore, Tstore, Tstore,
		Tstore, Tstore, Tstore, Tstore, Tstore,

		// 0x4F a 0x56
		Tastore, Tastore, Tastore, Tastore, Tastore, Tastore, Tastore, Tastore,

		// 0x57 a 0x5F
		 handleStack, handleStack, handleStack, handleStack, handleStack, handleStack, handleStack, handleStack, handleStack,

		// 0x60 a 0x63
		Tadd, Tadd, Tadd, Tadd,

		// 0x64 a 0x67
		Tsub, Tsub, Tsub, Tsub,

		// 0x68 a 0x6N
		Tmul, Tmul, Tmul, Tmul,

		// 0x6C a 0x6F
		Tdiv, Tdiv, Tdiv, Tdiv,

		// 0x70 a 0x73
		Trem, Trem, Trem, Trem,

		// 0x74 a 0x77
		Tneg, Tneg, Tneg, Tneg,

		// 0x78 e 0x79
		Tshl, Tshl,

		// 0x7A e 0x7B
		Tshr, Tshr,

		// 0x7C e 0x7D
		Tushr, Tushr,

		// 0x7E e 0x7F
		Tand, Tand,

		// 0x80 e 0x81
		Tor, Tor,

		// 0x82 e 0x83
		Txor, Txor,

		// 0x84
		Tinc,

		// 0x85 a 0x87
		i2T, i2T, i2T,

		// 0x88 a 0x8A
		l2T, l2T, l2T,

		// 0x8B a 0x8D
		f2T, f2T, f2T,

		// 0x8E a 0x90
		d2T, d2T, d2T,

		// 0x91 a 0x93
		i2T, i2T, i2T,

		// 0x94
		Tcmp,

		// 0x95 a 0x98
		TcmpOP, TcmpOP, TcmpOP, TcmpOP,

		// 0x99 a 0x9E
		ifOP, ifOP, ifOP, ifOP, ifOP, ifOP,

		// 0x9F a 0xA4
		if_icmOP, if_icmOP, if_icmOP, if_icmOP, if_icmOP, if_icmOP,

		// 0xA5 e 0xA6
		if_acmOP, if_acmOP,

		// 0xA7, 0xA8, 0xA9
		jump, jump, jump,

		// 0xAA e 0xAB
		switch_, switch_,

		// 0xAC a 0xB1
		Treturn, Treturn, Treturn, Treturn, Treturn, Treturn,

		// 0xB2 a 0xB5
		accessField, accessField, accessField, accessField,

		// 0xB6 a 0xBA
		invoke, invoke, invoke, invoke, invoke,

		// 0xBB a 0xBE
		handleObject, handleObject, handleObject, handleObject,

		// 0xBF
		athrow_,

		// 0xC0 e 0xC1
		properties, properties,

		// 0xC2 e 0xC3
		monitor, monitor,

		// 0xC4
		wide_,

		// 0xC5
		handleObject,

		// 0xC6 e 0xC7
		ifNull, ifNull,

		// 0xC8 a 0xC9
		widejump, widejump,

		// 0xCA
		breakpoint_,

		// 0xCB a 0xFD
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined,	nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined,	nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined, nonDefined, nonDefined, nonDefined,
		nonDefined, nonDefined,	nonDefined, nonDefined, nonDefined,
		nonDefined,

		// 0xFE e 0xFF
		impdep, impdep
};


// Array contendo strings com os mnemônicos da JVM, para utilização no Exibidor.
char	*	opcodesJVM[] = {
		// 0 a 9
		"nop", "aconst_null", "iconst_m1", "iconst_0", "iconst_1", "iconst_2", "iconst_3", "iconst_4", "iconst_5", "lconst_0",
		// 10 a 19
		"lconst_1", "fconst_0", "fconst_1", "fconst_2", "dconst_0", "dconst_1", "bipush", "sipush", "ldc", "ldc_w",
		// 20 a 29
		"ldc2_w", "iload", "lload", "fload","dload", "aload", "iload_0", "iload_1", "iload_2", "iload_3",
		// 30 a 39
		"lload_0", "lload_1", "lload_2", "lload_3", "fload_0", "fload_1", "fload_2", "fload_3", "dload_0", "dload_1",
		// 40 a 49
		"dload_2", "dload_3", "aload_0", "aload_1", "aload_2", "aload_3", "iaload", "laload", "faload", "daload",
		// 50 a 59
		"aaload", "baload", "caload", "saload", "istore", "lstore", "fstore", "dstore", "astore", "istore_0",
		// 60 a 69
		"istore_1", "istore_2", "istore_3", "lstore_0", "lstore_1", "lstore_2", "lstore_3", "fstore_0", "fstore_1", "fstore_2",
		// 70 a 79
		"fstore_3", "dstore_0", "dstore_1", "dstore_2", "dstore_3", "astore_0", "astore_1", "astore_2", "astore_3", "iastore",
		// 80 a 89
		"lastore", "fastore", "dastore", "aastore", "bastore", "castore", "sastore", "pop", "pop2", "dup",
		// 90 a 99
		"dup_x1", "dup_x2", "dup2", "dup2_x1", "dup2_x2", "swap", "iadd", "ladd", "fadd", "dadd",
		// 100 a 109
		"isub", "lsub", "fsub", "dsub", "imul", "lmul", "fmul", "dmul", "idiv", "ldiv",
		// 110 a 119
		"fdiv", "ddiv", "irem", "lrem", "frem", "drem", "ineg", "lneg", "fneg", "dneg",
		// 120 a 129
		"ishl", "lshl", "ishr", "lshr", "iushr", "lushr", "iand", "land", "ior", "lor",
		// 130 a 139
		"ixor", "lxor", "iinc", "i2l", "i2f", "i2d", "l2i", "l2f", "l2d", "f2i",
		// 140 a 149
		"f2l", "f2d", "d2i", "d2l", "d2f", "i2b", "i2c", "i2s", "lcmp", "fcmpl",
		// 150 a 159
		"fcmpg", "dcmpl", "dcmpg", "ifeq", "ifne", "iflt", "ifge","ifgt", "ifle", "if_icmpeq",
		// 160 a 169
		"if_icmpne", "if_icmplt", "if_icmpge", "if_icmpgt", "if_icmple", "if_acmpeq", "if_acmpne", "goto", "jsr", "ret",
		// 170 a 179
		"tableswitch", "lookupswitch", "ireturn", "lreturn", "freturn", "dreturn", "areturn", "return", "getstatic", "putstatic",
		// 180 a 189
		"getfield", "putfield", "invokevirtual", "invokespecial", "invokestatic", "invokeinterface", "invokedynamic", "new", "newarray", "anewarray",
		// 190 a 199
		"arraylength", "athrow", "checkcast", "instanceof", "monitorenter", "monitorexit", "wide", "multianewarray", "ifnull", "ifnonnull",
		// 200 a 209
		"goto_w", "jsr_w", "breakpoint", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		// 210 a 219
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		// 220 a 229
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		// 230 a 239
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		// 240 a 249
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		// 250 a 255
		NULL, NULL, NULL, NULL, "impdep1", "impdep2"
};
