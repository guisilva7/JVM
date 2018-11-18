// codigo baseado em:
// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>	

#define		CLASS			1
#define		METHOD_FIELD	2
#define		ATTRIBUTE		3

#define		T_BOOLEAN		4
#define		T_CHAR			5
#define		T_FLOAT			6
#define		T_DOUBLE		7
#define		T_BYTE			8
#define		T_SHORT			9
#define		T_INT			10
#define		T_LONG			11

// 4.4. The Constant Pool 
// All constant_pool table entries have the following general format
// cp_info {
//     u1 tag;
//     u1 info[]; // cada item da constant_pool tem um info[] proprio, definido como item_info
// }

// Table 4.3. Constant pool tags
#define CONSTANT_Class 					7
#define CONSTANT_Fieldref 				9
#define CONSTANT_Methodref 				10
#define CONSTANT_InterfaceMethodref 	11
#define CONSTANT_String 				8
#define CONSTANT_Integer 				3
#define CONSTANT_Float 					4
#define CONSTANT_Long 					5
#define CONSTANT_Double 				6
#define CONSTANT_NameAndType 			12
#define CONSTANT_Utf8 					1
// #define CONSTANT_MethodHandle 			15
// #define CONSTANT_MethodType 			16
// #define CONSTANT_InvokeDynamic 			18

// 4.4.1. The CONSTANT_Class_info Structure 
// 4.4.2. The CONSTANT_Fieldref_info, CONSTANT_Methodref_info, and CONSTANT_InterfaceMethodref_info Structures 
// 4.4.3. The CONSTANT_String_info Structure 
// 4.4.4. The CONSTANT_Integer_info and CONSTANT_Float_info Structures 
// 4.4.5. The CONSTANT_Long_info and CONSTANT_Double_info Structures 
// 4.4.6. The CONSTANT_NameAndType_info Structure
// 4.4.7. The CONSTANT_Utf8_info Structure 
// 4.4.8. The CONSTANT_MethodHandle_info Structure 
// 4.4.9. The CONSTANT_MethodType_info Structure 
// 4.4.10. The CONSTANT_InvokeDynamic_info Structure 


// Estrutura para representar a constante
// typedef struct 
// {
// 	u1 tag;
// 	union 
// 	{
//    		struct 
//    		{
//       		u2 name_index;
// 		} Class;
		
// 		struct 
// 		{
// 		      u2 class_index;
// 		      u2 name_and_type_index;
// 		} Fieldref;
// 		...
typedef struct constant_pool_info
{
	uint8_t	tag; // representa o numero do CONSTANT_X
	union
	{
		struct{									// 4.4.1
			uint16_t	name_index;				
		}Class;

		struct{									// 4.4.2
			// class_index;
			uint16_t	name_index;				
			uint16_t	name_and_type_index;
		}Ref;

		struct{									// 4.4.3
			uint16_t	string_index;
		}String;

		struct{									// 4.4.4
			uint32_t	bytes;
		}Integer_Float;

		struct{									// 4.4.5
			uint32_t 	high_bytes;
			uint32_t 	low_bytes;
		}Long_Double;

		struct{									// 4.4.6			
			uint16_t	name_index;
			uint16_t	descriptor_index;
		}NameAndType;

		struct{									// 4.4.7
			uint16_t	length;
			uint8_t*	bytes;
		}Utf8;


	}u;
}constant_pool_info;

// 4.7. Attributes 
// Attributes are used in the ClassFile, field_info, method_info, and Code_attribute 
// structures of the class file format
// All attributes have the following general format
// attribute_info {
//     u2 attribute_name_index;
//     u4 attribute_length;
//     u1 info[attribute_length]; // cada item de atributo tem um info[] proprio, definido como item_attribute
// }

// Table 4.6. Predefined class file attributes
// estes valores foram escolhidos por nos e serao utilizados num switch na funcao getttributeType
// de acordo com attribute_name_index selecionamos o tipo na constant_pool
#define		CONSTANT_VALUE			0
#define		CODE					1
#define		DEPRECATED				2
#define		EXCEPTIONS				3
#define		INNER_CLASSES			4
#define		LINE_NUMBER_TABLE		5
#define		LOCAL_VARIABLE_TABLE	6
#define		SYNTHETIC				7
#define		SOURCE_FILE				8
#define		UNKNOWN					9
#define		ATTRIBUTE_TYPE			short

// 4.7.2. The ConstantValue Attribute 
// 4.7.3. The Code Attribute 
// 4.7.4. The StackMapTable Attribute 
// 4.7.5. The Exceptions Attribute 
// 4.7.6. The InnerClasses Attribute 
// 4.7.7. The EnclosingMethod Attribute 
// 4.7.8. The Synthetic Attribute 
// 4.7.9. The Signature Attribute 
// 4.7.10. The SourceFile Attribute 
// 4.7.11. The SourceDebugExtension Attribute 
// 4.7.12. The LineNumberTable Attribute 
// 4.7.13. The LocalVariableTable Attribute 
// 4.7.14. The LocalVariableTypeTable Attribute 
// 4.7.15. The Deprecated Attribute 
// 4.7.16. The RuntimeVisibleAnnotations attribute 
// 4.7.17. The RuntimeInvisibleAnnotations attribute 	
// 4.7.18. The RuntimeVisibleParameterAnnotations attribute 
// 4.7.19. The RuntimeInvisibleParameterAnnotations attribute 
// 4.7.20. The AnnotationDefault attribute 
// 4.7.21. The BootstrapMethods attribute 

// 4.7.3. The Code Attribute 
typedef struct exception_table_type{
	uint16_t	start_pc;
	uint16_t	end_pc;
	uint16_t	handler_pc;
	uint16_t	catch_type;
}exception_table_type;

// 4.7.6. The InnerClasses Attribute
typedef struct classes_type{
	uint16_t	inner_class_info_index;
	uint16_t	outer_class_info_index;
	uint16_t	inner_name_index;
	uint16_t	inner_class_access_flags;
}classes_type;

// 4.7.12. The LineNumberTable Attribute 
typedef struct line_number_table_type{
	uint16_t	start_pc;
	uint16_t	line_number;
}line_number_table_type;

// 4.7.13. The LocalVariableTable Attribute
typedef struct local_variable_table_type{
	uint16_t	start_pc;
	uint16_t	length;
	uint16_t	name_index;
	uint16_t	descriptor_index;
	uint16_t	index;
}local_variable_table_type;

typedef struct attribute_info{
	// The constant_pool entry at that index must be a CONSTANT_Utf8_info of attribute name
	uint16_t	attribute_name_index;
	uint32_t	attribute_length;
	union{
		struct{											// 4.7.2
			uint16_t	constantvalue_index;
		}ConstantValue;

		struct{											// 4.7.3
			uint16_t		max_stack;
			uint16_t 		max_locals;
			uint32_t		code_length;
			uint8_t*		code;
			uint16_t		exception_table_length;
			exception_table_type *	exception_table;
			uint16_t		attributes_count;
			struct attribute_info*	attributes;
		}Code;

		struct{											// 4.7.15
		}Deprecated;

		struct{											// 4.7.5
			uint16_t		number_of_exceptions;
			uint16_t*		exception_index_table;
		}Exceptions;

		struct{											// 4.7.6
			uint16_t		number_of_classes;
			classes_type *	classes;
		}InnerClasses;

		struct{											// 4.7.12
			uint16_t		line_number_table_length;
			line_number_table_type *	line_number_table;
		}LineNumberTable;

		struct{
			// local_variable_type_table_length
			uint16_t		local_variable_table_length; // 4.7.14
			local_variable_table_type *	local_variable_table;
		}LocalVariableTable;

		struct{											// 4.7.8
		}Synthetic;

		struct{											// 4.7.10
			uint16_t		sourcefile_index;
		}SourceFile;
	}u;
}attribute_info;

// 4.5. Fields
// Each field is described by a field_info structure. No two fields in one class 
// file may have the same name and descriptor (§4.3.2). 
typedef	struct field_info{
	uint16_t		access_flags;			// máscara de bits que especifica permissões de acesso e propriedades do field
	uint16_t		name_index;				// índice para o contant_pool contendo o nome do field
	uint16_t		descriptor_index;		// índice para o contant_pool contendo um descritor de field válido
	uint16_t		attributes_count;		// número de atributos do field
	attribute_info*	attributes;				// array contendo informações sobre os atributos do field
}field_info;

// 4.6. Methods 
// Each method, including each instance initialization method (§2.9) and the class or interface initialization 
// method (§2.9), is described by a method_info structure. No two methods in one class file may have the same 
// name and descriptor (§4.3.3). 
typedef struct method_info{
	uint16_t		access_flags;			// máscara de bits que especifica permissões de acesso e propriedades do método 
	uint16_t		name_index;			// índice para o contant_pool contendo o nome simples do método, ou nome especial <init> 
	uint16_t		descriptor_index;		// índice para o contant_pool contendo um descritor de método válido
	uint16_t		attributes_count;		// número de atributos do metodo
	attribute_info*	attributes;
}method_info;

// 4.1. The ClassFile Structure 
// class file consists of a single ClassFile structure
typedef struct ClassFile{
	uint32_t		magic;					// assinatura do arquivo .class = 0xCAFEBABE
	uint16_t		minor_version;			// (m)
	uint16_t		major_version;			// (M)	indicam a versao do arquivo no formato M.m
											//4 bytes, versão para comparar com o que a JVM pode ler.
	uint16_t		constant_pool_count;	// número de entradas da tabela constant_pool + 1 
											// Cada array de elemento contem um byte que representa o tipo da contant_pool (string, char, int, etc)
	constant_pool_info*	constant_pool;		// tabela de estruturas representando strings, nomes de classes, interfaces, nomes de campos, etc.
	uint16_t		access_flags;			// máscara de bits que especifica permissões de acesso e propriedades da classe ou interface
	uint16_t		this_class;				// aponta para uma estrutura CONSTANT_Class_info. Representa a classe ou interface definida pelo ClassFile 
	uint16_t		super_class;			// aponta para uma estrutura CONSTANT_Class_info. Representa a superclasse direta (classe mãe)
	uint16_t		interfaces_count;		// número de entradas no array 'interfaces'
	uint16_t*		interfaces;				// cada entrada desse array é um índice da constant_pool do tipo CONSTANT_Class_info, representa uma interface que é uma superinterface direta desta classe ou interface

	uint16_t		fields_count;			// número de entradas no array 'fields'
	field_info *	fields;					// cada entrada desse array é uma estrutura 'field_info'. Variaveis de classe ou de instâncias declaradas nesta classe ou interface

	uint16_t		methods_count;			// número de entradas no array 	'methods'
	method_info*	methods;				// cada entrada desse array é uma estrutura 'method_info', contendo a descrição completa de um método da classe ou interface
	uint16_t		attributes_count;		// número de entradas no array 'attributes'
	attribute_info*	attributes;				// cada entrada desse array é uma estrutura 'attribute_info'
}ClassFile;

// Table 4.1. Class access and property modifiers:
// ACC_PUBLIC, 										 , ACC_FINAL, ACC_SUPER,                                                ACC_INTERFACE, ACC_ABSTRACT,               ACC_SYNTHETIC,  ACC_ANNOTATION, ACC_ENUM 
// Table 4.4. Field access and property flags:
// ACC_PUBLIC, ACC_PRIVATE, ACC_PROTECTED, ACC_STATIC, ACC_FINAL,                   ACC_VOLATILE, ACC_TRANSIENT,                                                       ACC_SYNTHETIC,                  ACC_ENUM 
// Table 4.5. Method access and property flags: 
// ACC_PUBLIC, ACC_PRIVATE, ACC_PROTECTED, ACC_STATIC, ACC_FINAL, ACC_SYNCHRONIZED, ACC_BRIDGE,   ACC_VARARGS,   ACC_NATIVE,               ACC_ABSTRACT, ACC_STRICT,   ACC_SYNTHETIC
#define		ACC_PUBLIC		0x1 	// Declared public; may be accessed from outside its package. 

#define	    ACC_PRIVATE 	0x0002 	// Declared private; usable only within the defining class.
#define	    ACC_PROTECTED 	0x0004 	// Declared protected; may be accessed within subclasses.
#define		ACC_STATIC 		0x0008 	// Declared static.

#define		ACC_FINAL		0x10	// Declared final; no subclasses allowed. 	
#define		ACC_SUPER		0x20 	// Treat superclass methods specially when invoked by the invokespecial instruction.

#define		ACC_SYNCHRONIZED 0x0020 // Declared synchronized; invocation is wrapped by a monitor use.
#define		ACC_BRIDGE 		0x0040 	// A bridge method, generated by the compiler.
#define		ACC_VARARGS 	0x0080 	// Declared with variable number of arguments.


#define		ACC_VOLATILE 	0x0040 	// Declared volatile; cannot be cached.
#define		ACC_TRANSIENT 	0x0080 	// Declared transient; not written or read by a persistent object manager.

#define     ACC_NATIVE 		0x0100 	// Declared native; implemented in a language other than Java.

#define		ACC_INTERFACE	0x200	// Is an interface, not a class.
#define		ACC_ABSTRACT	0x400 	// Declared abstract; must not be instantiated. 
#define		ACC_STRICT 		0x0800 	// Declared strictfp; floating-point mode is FP-strict.
// Not implemented
#define		ACC_SYNTHETIC 	0x1000 	// Declared synthetic; not present in the source code.
#define		ACC_ANNOTATION 	0x2000 	// Declared as an annotation type.
#define		ACC_ENUM 		0x4000 	// Declared as an enum type. 




// Methods
// para ler bytes do arquivo
uint8_t	u1Read(FILE *);
uint16_t u2Read(FILE *);
uint32_t u4Read(FILE *);

// para obter tipo do atributo
ATTRIBUTE_TYPE	getAttributeType(attribute_info *, ClassFile *);

// obtem campos do ClassFile
ClassFile * obtainClassFile(FILE *);
void obtainConstantPool(ClassFile *, FILE *);
void obtainInterfaces(ClassFile *, FILE *);
void obtainFields(ClassFile *, FILE *);
void obtainMethods(ClassFile *, FILE *);
void obtainAttributes(field_info *, method_info *, attribute_info *, ClassFile *, FILE *);

// mostra ClassFile no arquivo de saida
void showClassFile(ClassFile *, FILE *);
void showGeneralInfo(ClassFile *, FILE *);
void showConstantPool(ClassFile *, FILE *);
void showInterfaces(ClassFile *, FILE *);
void showFields(ClassFile *, FILE *);
void showMethods(ClassFile *, FILE *);
void showAttributes(field_info *, method_info *, attribute_info *, ClassFile *, FILE *);	// atributos de field, metodo, de Code_Attribute ou de classe.

// mostra constantes no arquivo de saida
void printConstUtf8(constant_pool_info *, FILE *);
void printConstantClass(ClassFile *,constant_pool_info *, FILE *);
void printConstantString(ClassFile *, constant_pool_info *, FILE *);
void printConstantNameAndType(ClassFile *, constant_pool_info *, char, FILE *);
void printConstantRef(ClassFile *,constant_pool_info *, char, FILE *);

// limpa cada estrutura
void unloadClassFile(ClassFile *);
void unloadConstantPool(ClassFile *);
void unloadFields(ClassFile *);
void unloadMethods(ClassFile *);
void unloadAttribute(attribute_info *, ClassFile *);



