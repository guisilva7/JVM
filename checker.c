#include "checker.h"

/*==========================================*/
// função VerBytecode
void VerBytecode(attribute_info *attr, ClassFile *arq) {

}  // fim da funcao VerBytecode



/*==========================================*/
// função DescritorCampo
bool DescritorCampo(uint16_t index, constant_pool_info *cp) {
	uint16_t comprimento = cp->u.Utf8.length;
	uint8_t *bytes = cp->u.Utf8.bytes + index;
	if (index == comprimento) {
		return true;
	}
	switch (*bytes) {
		case BOOLEAN:
		case BYTE:
		case CHAR:
		case DOUBLE:
		case FLOAT:
		case INT:
		case LONG:
		case SHORT:
		case REF_ARRAY:
			return DescritorCampo(++index,cp);
			break;
		case REF_INST:;
			char *string = (char *)bytes;
			string[cp->u.Utf8.length - index] = '\0';

			if ((comprimento - index) < 3 || !strchr(string, ';')) {
				return false;
			}
			index += strlen(string);
			return DescritorCampo(index, cp);
			break;
		default:
			return false;
	}
	return false;
}
// fim da função DescritorCampo

/*==========================================*/
// função DescritorMetodo
bool DescritorMetodo(constant_pool_info *cp, uint16_t index) {
	uint16_t comprimento = cp->u.Utf8.length;
	uint8_t *bytes = cp->u.Utf8.bytes + index;

	/*	printf("index = %" PRIuint8_t6 " comprimento = %" PRIuint8_t6 "\n", index,
	 * length);*/
	if (index == comprimento) {
		return true;
		;
	}

	bool returnDescriptor = false;
	if ((*bytes) == ')') {
		returnDescriptor = true;
		index++;
		bytes++;
		while (((*bytes) == '[')) {
			index++;
			bytes++;
		}
		if (index == comprimento) {
			puts("erro tipo retorno");
			return false;
		}
	}
	switch (*bytes) {
		case BOOLEAN:
		case BYTE:
		case CHAR:
		case DOUBLE:
		case FLOAT:
		case INT:
		case LONG:
		case SHORT:
		case REF_ARRAY:
			if (!returnDescriptor) {
				return DescritorMetodo(cp, ++index);
			}
			if (index == (comprimento - 1)) {
				return true;
			}
			puts("erro tipo retorno");
			return false;
			break;
		case REF_INST:;
			char *string = (char *)bytes;
			string[cp->u.Utf8.length - index] = '\0';


			if ((comprimento - index) < 3 || !strchr(string, ';')) {
				puts("Erro ref inst");
				return false;
			}
			index += strlen(string);
			if (!returnDescriptor) {
				return DescritorMetodo(cp, index);
			}
			if (index == comprimento) {
				return true;
			}
			/*			printConstUtf8(cp, stdout);*/
			puts("erro ref inst retorno");
			return false;
			break;
		case 'V':
			if (!returnDescriptor) {
				puts("erro void parametro");
				return false;
			}
			if (index == (comprimento - 1)) {
				return true;
			}
			puts("erro void retorno");
			return false;
			break;
		default:
			printf("index = %" PRIu16 " comprimento = %" PRIu16 "\n",
				   index, comprimento);
			printf("%c\n", *bytes);
			printConstUtf8(cp, stdout);
			puts("erro tipo desconhecido");
			return false;
	}
}
// fim da função DescritorCampo


/*==========================================*/
// função AcessoFlags
void AcessoFlags(ClassFile *arq) {
	uint16_t access_flags;

	// Testando flags de classes
	access_flags = arq->access_flags;
	if (access_flags & 0xF9CE) {
		puts("erro de acesso a flags de classe");
		exit(EXIT_FAILURE);
	}

	// Testando flags de fields
	for (uint16_t i = 0; i < arq->fields_count; i++) {
		access_flags = (arq->fields + i)->access_flags;
		if (access_flags & 0xFF20) {
			puts("erro de acesso a flags de fields");
			exit(EXIT_FAILURE);
		}
	}

	// Testando flags de métodos
	for (uint16_t i = 0; i < arq->methods_count; i++) {
		access_flags = (arq->methods + i)->access_flags;
		if (access_flags & 0xF2C0) {
			puts("erro de acesso a flags de method");
			exit(EXIT_FAILURE);
		}
	}
}  // fim da função verifyAccessFlag



/*==========================================*/
// função ArquivoClassverificador
void ArquivoClassverificador(ClassFile *arq) {
	// checa inconsistências na Constant Pool
	verifyConstantPool(arq);

	// checa flags inválidas
	AcessoFlags(arq);

	constant_pool_info *cp_auxiliar;

	// checa this_class
	cp_auxiliar = arq->constant_pool + arq->this_class - 1;
	if (cp_auxiliar->tag != CONSTANT_Class) {
		puts("erro indice this_class invalido");
		exit(EXIT_FAILURE);
	}

	// checa super_class
	if (arq->super_class) {
		cp_auxiliar = arq->constant_pool + arq->super_class - 1;
		if (cp_auxiliar->tag != CONSTANT_Class) {
			puts("erro  indice de superclasse invalido");
			exit(EXIT_FAILURE);
		}
	} else {
		// Toda classe, exceto Object, deve ter uma superclasse FALTA TESTAR
		// ISSO
		cp_auxiliar = arq->constant_pool + arq->this_class - 1;
		cp_auxiliar = arq->constant_pool + cp_auxiliar->u.Class.name_index - 1;
		char *string = (char *)cp_auxiliar->u.Utf8.bytes;
		string[cp_auxiliar->u.Utf8.length] = '\0';
		if (strcmp(string, "java/lang/Object")) {
			puts("erro nao possui super_class");
			exit(EXIT_FAILURE);
		}
	}

	// checa interfaces
	for (uint16_t i = 0; i < arq->interfaces_count; i++) {
		cp_auxiliar = arq->constant_pool + (*(arq->interfaces + i)) - 1;
		if (cp_auxiliar->tag != CONSTANT_Class) {
			puts("erro interface invalidoa");
			exit(EXIT_FAILURE);
		}
	}

	// checa fields
	for (uint16_t i = 0; i < arq->fields_count; i++) {
		uint16_t name_index = (arq->fields + i)->name_index;
		cp_auxiliar = arq->constant_pool + name_index - 1;
		if (cp_auxiliar->tag != CONSTANT_Utf8) {
			puts("erro  field name_index");
			exit(EXIT_FAILURE);
		}
		uint16_t descriptor_index = (arq->fields + i)->descriptor_index;
		cp_auxiliar = arq->constant_pool + descriptor_index - 1;
		if (cp_auxiliar->tag != CONSTANT_Utf8) {
			puts("erro  field descriptor_index");
			exit(EXIT_FAILURE);
		}
		// checa atributos de fields
		uint16_t attributes_count = (arq->fields + i)->attributes_count;
		for (uint16_t j = 0; j < attributes_count; j++) {
			cp_auxiliar = arq->constant_pool +
					 ((arq->fields + i)->attributes + j)->attribute_name_index -
					 1;
			if (cp_auxiliar->tag != CONSTANT_Utf8) {
				puts("erro  field attribute_indicador_nome");
				exit(EXIT_FAILURE);
			}
			switch (getAttributeType(((arq->fields + i)->attributes + j), arq)) {
				case CODE:
				case EXCEPTIONS:
				case INNER_CLASSES:
				case LINE_NUMBER_TABLE:
				case LOCAL_VARIABLE_TABLE:
				case SOURCE_FILE:
					puts("erro atributo de field invalido");
					exit(EXIT_FAILURE);
					break;
				case DEPRECATED:
				case SYNTHETIC:
					if (((arq->fields + i)->attributes + j)->attribute_length) {
						puts("erro atributo invalido");
						exit(EXIT_FAILURE);
					}
					break;
			}
		}
	}

	// checa métodos
	for (uint16_t i = 0; i < arq->methods_count; i++) {
		uint16_t name_index = (arq->methods + i)->name_index;
		cp_auxiliar = arq->constant_pool + name_index;
		if (cp_auxiliar->tag != CONSTANT_Utf8) {
			puts("erro method name_index invalido ");
			exit(EXIT_FAILURE);
		}

		uint16_t descriptor_index = (arq->methods + i)->descriptor_index;
		cp_auxiliar = arq->constant_pool + descriptor_index;
		if (cp_auxiliar->tag != CONSTANT_Utf8) {
			puts("erro method descriptor_index invalido ");
			exit(EXIT_FAILURE);
		}

		// checa atributos de métodos
		uint16_t attributes_count = (arq->methods + i)->attributes_count;
		for (uint16_t j = 0; j < attributes_count; j++) {
			cp_auxiliar = arq->constant_pool +
					 ((arq->methods + i)->attributes + j)->attribute_name_index -
					 1;
			if (cp_auxiliar->tag != CONSTANT_Utf8) {
				puts("erro attribute_indicador_nome invalido");
				exit(EXIT_FAILURE);
			}
			switch (getAttributeType(((arq->methods + i)->attributes + j), arq)) {
				case CODE:
					VerBytecode(((arq->methods + i)->attributes + j), arq);
					// checa atributos do atributo Code
					uint16_t code_attributes_count =
						((arq->methods + i)->attributes +
						 j)->u.Code.attributes_count;
					for (uint16_t k = 0; k < code_attributes_count; k++) {
						attribute_info *attr = ((arq->methods + i)->attributes +
												j)->u.Code.attributes +
											   k;
						cp_auxiliar = arq->constant_pool +
								 attr->attribute_name_index - 1;
						if (cp_auxiliar->tag != CONSTANT_Utf8) {
							puts("erro attribute_indicador_nome invalido ");
							exit(EXIT_FAILURE);
						}
						switch (getAttributeType(attr, arq)) {
							case CODE:
							case DEPRECATED:
							case EXCEPTIONS:
							case INNER_CLASSES:
							case SOURCE_FILE:
							case SYNTHETIC:
								puts("erro codigo do atributo invalido");
								exit(EXIT_FAILURE);
								break;
							case LINE_NUMBER_TABLE:
								break;
							case LOCAL_VARIABLE_TABLE:
								for (uint16_t l = 0;
									 l < attr->u.LocalVariableTable
											 .local_variable_table_length;
									 l++) {
									cp_auxiliar = arq->constant_pool +
											 (attr->u.LocalVariableTable
												  .local_variable_table)
												 ->name_index;
									if (cp_auxiliar->tag != CONSTANT_Utf8) {
										puts("erro local invalido "
											 "variable table name_index");
										exit(EXIT_FAILURE);
									}
									cp_auxiliar = arq->constant_pool +
											 (attr->u.LocalVariableTable
												  .local_variable_table)
												 ->descriptor_index;
									if (cp_auxiliar->tag != CONSTANT_Utf8) {
										puts("erro  local invalido "
											 "variable table descriptor_index");
										exit(EXIT_FAILURE);
									}
									if (!DescritorCampo(0,cp_auxiliar)) {
										puts("erro field invalido  "
											 "descriptor");
										exit(EXIT_FAILURE);
									}
								}
								break;
						}
					}
					break;
				case EXCEPTIONS:;
					uint16_t number_of_exceptions =
						((arq->methods + i)->attributes +
						 j)->u.Exceptions.number_of_exceptions;
					for (uint16_t k = 0; k < number_of_exceptions; k++) {
						uint16_t exception_index =
							*(((arq->methods + i)->attributes +
							   j)->u.Exceptions.exception_index_table +
							  k);
						if (!exception_index) {
							puts("erro indice invalido");
							exit(EXIT_FAILURE);
						}
						cp_auxiliar = arq->constant_pool + exception_index - 1;
						if (cp_auxiliar->tag != CONSTANT_Class) {
							puts("erro indice invalido");
							exit(EXIT_FAILURE);
						}
					}
					break;
				case INNER_CLASSES:
				case LINE_NUMBER_TABLE:
				case LOCAL_VARIABLE_TABLE:
				case SOURCE_FILE:
					puts("erro atributo de metodo invalido");
					exit(EXIT_FAILURE);
					break;
				case DEPRECATED:
				case SYNTHETIC:
					if (((arq->methods + i)->attributes + j)->attribute_length) {
						puts("erro atributo de metodo invalido");
						exit(EXIT_FAILURE);
					}
					break;
			}
		}
	}

	// checa atributos de classe
	for (uint16_t i = 0; i < arq->attributes_count; i++) {
		cp_auxiliar = arq->constant_pool +
				 (arq->attributes + i)->attribute_name_index - 1;
		if (cp_auxiliar->tag != CONSTANT_Utf8) {
			puts("erro  invalido class attribute_indicador_nome");
			exit(EXIT_FAILURE);
		}
		switch (getAttributeType(arq->attributes + i, arq)) {
			case CODE:
			case EXCEPTIONS:
			case LINE_NUMBER_TABLE:
			case LOCAL_VARIABLE_TABLE:
				puts("erro atributo de classe invalido");
				exit(EXIT_FAILURE);
				break;
			case INNER_CLASSES:;
				uint16_t number_of_classes =
					(arq->attributes + i)->u.InnerClasses.number_of_classes;
				for (uint16_t j = 0; j < number_of_classes; j++) {
					classes_type *class =
						((arq->attributes + i)->u.InnerClasses.classes + j);
					if (!(class->inner_class_info_index) ||
						!(class->outer_class_info_index) ||
						!(class->inner_name_index)) {
						puts("erro de classe invalido");
						exit(EXIT_FAILURE);
					}
					cp_auxiliar =
						arq->constant_pool + class->inner_class_info_index - 1;
					if (cp_auxiliar->tag != CONSTANT_Class) {
						puts("erro  invalido: inner_class_info_index");
						exit(EXIT_FAILURE);
					}
					cp_auxiliar =
						arq->constant_pool + class->outer_class_info_index - 1;
					if (cp_auxiliar->tag != CONSTANT_Class) {
						puts("erro  invalido: outer_class_info_index");
						exit(EXIT_FAILURE);
					}
					cp_auxiliar = arq->constant_pool + class->inner_name_index - 1;
					if (cp_auxiliar->tag != CONSTANT_Utf8) {
						puts("erro  invalido: inner_indicador_nome");
						exit(EXIT_FAILURE);
					}
					uint16_t access_flags = class->inner_class_access_flags;
					if (access_flags & 0xF9E0) {
						puts("erro  invalido: inner_class_access_flags");
						exit(EXIT_FAILURE);
					}
				}
				break;
			case SOURCE_FILE:
				if ((arq->attributes + i)->attribute_length != 2) {
					puts("erro atributo de arquivo fonte invalido ");
					exit(EXIT_FAILURE);
				}
				cp_auxiliar = arq->constant_pool +
						 (arq->attributes + i)->u.SourceFile.sourcefile_index -
						 1;
				if (cp_auxiliar->tag != CONSTANT_Utf8) {
					puts("erro  invalido: sourcefile_index");
					exit(EXIT_FAILURE);
				}
				break;
			case DEPRECATED:
			case SYNTHETIC:
				if ((arq->attributes + i)->attribute_length) {
					puts("erro atributo de classe invalido");
					exit(EXIT_FAILURE);
				}
				break;
		}
	}
}  // fim da funcao ArquivoClassverificador

/*==========================================*/
// função verifyOverrideMethodFinal
void VerificadorOverride(ClassFile *arq, JVM *jvm) {
	// verifica se método final é sobrecarregado
	constant_pool_info *cp_auxiliar = NULL;
	for (uint16_t count = 0; count < arq->methods_count; count++) {
        
		cp_auxiliar = (arq->methods + count)->name_index + arq->constant_pool - 1;
		char *nome_metodo = (char *)cp_auxiliar->u.Utf8.bytes;
		nome_metodo[cp_auxiliar->u.Utf8.length] = '\0';
		CLASS_DATA *cd_super = cd_super = getSuperClass(arq, jvm);
		while (cd_super) {
			for (uint16_t count = 0; count < (cd_super->classfile)->methods_count;
				 count++) {
				cp_auxiliar = ((cd_super->classfile)->methods + count)->name_index +
						 (cd_super->classfile)->constant_pool - 1;
				char *super_nome_metodo = (char *)cp_auxiliar->u.Utf8.bytes;
				super_nome_metodo[cp_auxiliar->u.Utf8.length] = '\0';
				if (!strcmp(nome_metodo, super_nome_metodo)) {
					if (((cd_super->classfile)->methods + count)->access_flags ==
						ACC_FINAL) {
						puts("Erro de override metodo final");
						exit(EXIT_FAILURE);
					}
				}
			}
			cd_super = getSuperClass(cd_super->classfile, jvm);
		}
	}
}

/*==========================================*/
// função verifyFinal
void SuperVerificador(ClassFile *arq, JVM *jvm) {
	// verifica se a classe atual foi herdada uma classe final
	CLASS_DATA *cd_super;
	if ((cd_super = getSuperClass(arq, jvm))) {
		if ((cd_super->classfile)->access_flags & ACC_FINAL) {
			puts("erro de herança de super classe final");
			exit(EXIT_FAILURE);
		}
	}
}
/*==========================================*/
// função verifyConstantPool
void verifyConstantPool(ClassFile *arq) {
	// Verify Constant Pool
	constant_pool_info *cp;
	for (uint16_t i = 0; i < (arq->constant_pool_count - 1); i++) {
		cp = arq->constant_pool + i;
		switch (cp->tag) {
			case CONSTANT_Class:
				if ((arq->constant_pool + cp->u.Class.name_index - 1)->tag !=
					CONSTANT_Utf8) {
					puts("erro do nome de index de classe");
					exit(EXIT_FAILURE);
				}
				break;
			case CONSTANT_Fieldref:
			case CONSTANT_Methodref:
			case CONSTANT_InterfaceMethodref:
				if ((arq->constant_pool + cp->u.Ref.name_index - 1)->tag !=
					CONSTANT_Class) {
					puts("erro: referencia ao indice de classe invalido");
					exit(EXIT_FAILURE);
				}
				if ((arq->constant_pool + cp->u.Ref.name_and_type_index -
					 1)->tag != CONSTANT_NameAndType) {
					puts("erro de name_and_type index");
					exit(EXIT_FAILURE);
				}
				if (cp->tag != CONSTANT_Fieldref) {
					constant_pool_info *cp_auxiliar;
					cp_auxiliar = arq->constant_pool + cp->u.Ref.name_and_type_index - 1;
					cp_auxiliar = arq->constant_pool + cp_auxiliar->u.NameAndType.descriptor_index - 1;
					if (cp_auxiliar->tag != CONSTANT_Utf8) {
						puts("erro de indice de descritor");
						exit(EXIT_FAILURE);
					}
					if ((cp_auxiliar->u.Utf8).length < 3) {
						puts("erro de indice de descritor");
						exit(EXIT_FAILURE);
					}
					if (((cp_auxiliar->u.Utf8).bytes)[0] != '(') {
						puts("erro de indice de descritor");
						exit(EXIT_FAILURE);
					}
				}
				break;
			case CONSTANT_String:
				if ((arq->constant_pool + cp->u.String.string_index -
					 1)->tag != CONSTANT_Utf8) {
					puts("indice de string invalidoa");
					exit(EXIT_FAILURE);
				}
				break;
			case CONSTANT_Integer:
			case CONSTANT_Float:
			case CONSTANT_Long:
			case CONSTANT_Double:
				break;
			case CONSTANT_NameAndType:
				if ((arq->constant_pool + cp->u.NameAndType.name_index -
					 1)->tag != CONSTANT_Utf8) {
					puts("erro indice NameAndType  invalido");
					exit(EXIT_FAILURE);
				}
				constant_pool_info *cp_auxiliar;
				if ((cp_auxiliar = arq->constant_pool +
							  cp->u.NameAndType.descriptor_index - 1)
						->tag != CONSTANT_Utf8) {
					puts("erro NameAndType descriptor_index invalido");
					exit(EXIT_FAILURE);
				}
				if (cp_auxiliar->u.Utf8.length == 0) {
					puts("erro desctitor invalido");
					exit(EXIT_FAILURE);
				}
				uint16_t comprimento = cp_auxiliar->u.Utf8.length;
				uint8_t *bytes = cp_auxiliar->u.Utf8.bytes;

				char *string = (char *)cp_auxiliar->u.Utf8.bytes;
				string[cp_auxiliar->u.Utf8.length] = '\0';
				switch (*bytes) {
					case BOOLEAN:
					case BYTE:
					case CHAR:
					case DOUBLE:
					case FLOAT:
					case INT:
					case LONG:
					case SHORT:
						if (comprimento != 1) {
							puts("erro  desctitor invalido");
							exit(EXIT_FAILURE);
						}
						break;
					case REF_INST:
						if (comprimento < 3 || bytes[comprimento - 1] != ';') {
							puts("erro  desctitor invalido");
							exit(EXIT_FAILURE);
						}
						break;
					case REF_ARRAY:
						if (comprimento < 2) {
							puts("erro  desctitor invalido");
							exit(EXIT_FAILURE);
						}
						uint16_t index = 0;
						while ((*bytes) == REF_ARRAY) {
							index++;
							bytes++;
						}
						if (!DescritorCampo(index, cp_auxiliar)) {
							puts("erro  desctitor invalido");
							exit(EXIT_FAILURE);
						}
						break;
					case '(':
						if (comprimento < 3) {
							puts("erro  desctitor invalido");
							exit(EXIT_FAILURE);
						}
						if (!DescritorMetodo(cp_auxiliar, 1)) {
							puts("erro  desctitor invalido");
							exit(EXIT_FAILURE);
						}
						break;
					default:
						puts("erro  desctitor invalido");
						exit(EXIT_FAILURE);
						break;
				}
			case CONSTANT_Utf8:
				break;
		}
	}
}  // fim da função verifyConstantPool
