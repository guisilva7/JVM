// iniciador da jvm, finalizador da jvm, (des)carregar classe, limpar objetos (frames, operandos e etc) 

#include "leitor_exibidor.h"
/*==========================================*/
//	TIPOS

typedef uint8_t OPCODE;
typedef char TYPE;
typedef uint32_t ARRAY_TYPE;

// Descritores de Fields
#define BYTE 'B'
#define CHAR 'C'
#define DOUBLE 'D'
#define FLOAT 'F'
#define INT 'I'
#define LONG 'J'
#define REF_INST 'L'
#define SHORT 'S'
#define BOOLEAN 'Z'
#define REF_ARRAY '['

// OBJECT
typedef struct object obj;
typedef struct array ar;


// 3.2 Data Types
typedef struct value 
{
	TYPE type;
	union {
		struct {  // BYTE
			int8_t byte;
		} Byte;

		struct {  // SHORT
			int16_t short_;
		} Short;

		struct {  // INTEGER
			int32_t integer;
		} Integer;

		struct {  // LONG
			uint32_t high_bytes;
			uint32_t low_bytes;
		} Long;

		struct {  // FLOAT
			uint32_t float_;
		} Float;

		struct {  // DOUBLE
			uint32_t high_bytes;
			uint32_t low_bytes;
		} Double;

		struct {  // CHAR
			uint16_t char_;
		} Char;

		struct {  // boolean
			uint8_t boolean;
		} Boolean;

		struct {  // INSTANCE REFERENCE
			struct object *reference;
		} InstanceReference;

		struct {  // ARRAY REFERENCE
			struct array *reference;
		} ArrayReference;

		struct {  // RETURN ADDRESS
			OPCODE *return_address;
		} ReturnAddress;
	} u;
} DATA_TYPES;

// 3.6.2 Operand Stacks
// OPERAND_STACK
// Each frame (§3.6) contains a last-in-first-out (LIFO) stack known as its operand stack. 
// The maximum depth of the operand stack of a frame is determined at compile time and is 
// supplied along with the code for the method associated with the frame (§4.7.3).

// The Java virtual machine supplies instructions to load constants or values from local variables
// or fields onto the operand stack. Other Java virtual machine instructions take operands from the 
// operand stack, operate on them, and push the result back onto the operand stack. The operand 
// stack is also used to prepare parameters to be passed to methods and to receive method results. 

// At any point in time an operand stack has an associated depth, where a value of type long or 
// double contributes two units to the depth and a value of any other type contributes one unit.
typedef struct operand 
{
	uint32_t value;
	TYPE type;
	struct operand *next;
} OPERAND_STACK;

// 3.6 Frames
// EH UMA PILHA DE FRAMES
// A frame is used to store data and partial results, as well as to perform dynamic linking , 
// return values for methods, and dispatch exceptions. 

// Each frame has its own array of local variables (§3.6.1), 
// its own operand stack (§3.6.2), 
// and a reference to the runtime constant pool (§3.5.5) 
// of the class of the current method. 
typedef struct frame 
{
	uint32_t *local_variables; 						// 3.6.1 Local Variables
	OPERAND_STACK *operand_stack; 					// 3.6.2 Operand Stacks
	constant_pool_info *current_constant_pool;
	struct frame *next;
} FRAME;


// THREAD
typedef struct thread 
{
	OPCODE * program_counter;  // https://docs.oracle.com/javase/specs/jvms/se6/html/Overview.doc.html#6648
	FRAME * jvm_stack;  		// https://docs.oracle.com/javase/specs/jvms/se6/html/Overview.doc.html#6654
	struct thread *next;
} THREAD;


// 3.5.3 Heap
// The Java virtual machine has a heap that is shared among all Java virtual 
// machine threads. The heap is the runtime data area from which memory for all 
// class instances and arrays is allocated. 
typedef struct heap_area 
{  // https://docs.oracle.com/javase/specs/jvms/se6/html/Overview.doc.html#15730
	struct object *objects;
	struct array *arrays;
} HEAP;

// FIELD_DATA
typedef struct field_data 
{
	constant_pool_info *field_name;		 // CONSTANTE_Utf8
	constant_pool_info *field_descriptor;  // CONSTANTE_Utf8
	TYPE field_type;
	uint16_t modifiers;  // access_flags
	field_info *info;
	struct variable *var;
} FIELD_DATA;

// VARIABLE
typedef struct variable 
{
	FIELD_DATA *field_reference;
	DATA_TYPES value;
	struct variable *next;
} VARIABLE;

// METHOD_DATA
typedef struct method_data {
	constant_pool_info *method_name;		  // CONSTANTE_Utf8
	constant_pool_info *method_descriptor;  // CONSTANTE_Utf8
	uint16_t modifiers;				  // access_flags
	struct CLASS_DATA *CLASS_DATA;
	// se o método não é abstrato.
	uint32_t code_length;
	uint8_t *bytecodes;	// instruções da jvm
	uint16_t stack_size;   // tamanho da pilha de operandos
	uint16_t locals_size;  // tamanho do vetor de variaveis locais
	uint16_t exception_table_length;
	exception_table_type *exception_table;
	method_info *info;
} METHOD_DATA;

// 3.5.4 Method Area
// The Java virtual machine has a method area that is shared among all Java virtual machine threads. 
// The method area is analogous to the storage area for compiled code of a conventional language or analogous 
// to the "text" segment in a UNIX process. 

// It stores per-class structures such as the runtime constant pool, field and method data, and the code for 
// methods and constructors, including the special methods (§3.9) used in class and instance initialization and 
// interface type initialization. 
typedef struct CLASS_DATA 
{
  constant_pool_info *class_name;  // CONSTANTE_Utf8
  constant_pool_info *runtime_constant_pool;
  struct CLASS_DATA *class_loader_reference;
  ClassFile *classfile;
  uint16_t modifiers;
  VARIABLE *class_variables;
  FIELD_DATA *field_data;
  METHOD_DATA *method_data;
  struct object *instance_class;
  struct CLASS_DATA *next;
} CLASS_DATA;

// 3.7 Representation of Objects
typedef struct object 
{
	CLASS_DATA *class_data_reference;
	VARIABLE *instance_variables;
	struct object *next;
} OBJECT;

typedef struct array 
{
	CLASS_DATA *class_data_reference;
	int32_t count;
	DATA_TYPES *entry;
	ARRAY_TYPE atype;
	struct array *next;
} ARRAY;

// JVM
// https://docs.oracle.com/javase/specs/jvms/se6/html/Overview.doc.html#6656
typedef struct jvm 
{
	THREAD *thread_jvm;
	HEAP *heap_jvm;
	CLASS_DATA * class_data_jvm;  // regiao da method area
} JVM;
/*==========================================*/


void startJVM(char *, int, char **);
void loadClass(char *, CLASS_DATA **, CLASS_DATA *, JVM *);
void linkClass(CLASS_DATA *, JVM *);
void verifyLink(CLASS_DATA *, JVM *);
void prepareLink(CLASS_DATA *, JVM *);
void resolveLink(ClassFile *, JVM *);
void initializeClass(CLASS_DATA *, JVM *, THREAD *);
void executeMethod(char *, char *, CLASS_DATA *, JVM *, THREAD *, void *, uint16_t, uint32_t *);

bool isSuperClass(CLASS_DATA *, CLASS_DATA *, JVM *);
void exitJVM(JVM *);

char 		   *getClassName(CLASS_DATA *);
CLASS_DATA 	   *getSuperClass(ClassFile *, JVM *);
CLASS_DATA 	   *getClass(constant_pool_info *, JVM *);
VARIABLE 	   *getClassVariable(constant_pool_info *, CLASS_DATA *);
VARIABLE 	   *getInstanceVariable(constant_pool_info *, OBJECT *);
METHOD_DATA    *getMethod(char *, char *, CLASS_DATA *, JVM *);
attribute_info *getCodeAttribute(METHOD_DATA *, CLASS_DATA *);

// descarrega estruturas
void unloadClass(CLASS_DATA *, JVM *);
void unloadFrame(FRAME *);
void unloadOperandStack(FRAME *);
void unloadThreads(THREAD *);
void unloadJvmStack(FRAME *);
void unloadMethodArea(CLASS_DATA *);
void unloadClassData(CLASS_DATA *);
void unloadHeap(HEAP *);
void unloadArrays(ARRAY *);
void unloadObjects(OBJECT *);
void unloadVariables(VARIABLE *);

// push e pop na pilha de operandos
void pushOperand(uint32_t, FRAME *);
uint32_t popOperand(FRAME *);
