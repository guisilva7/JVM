// validador de estruturas
#include "jvm.h"

bool DescritorCampo(uint16_t, constant_pool_info *);
bool DescritorMetodo(constant_pool_info *, uint16_t);
void VerificaConstantPool(ClassFile *);
void AcessoFlags(ClassFile *);
void VerificaBytecode(attribute_info *, ClassFile *);
void VerificadorArquivoClass(ClassFile *);
void SuperVerificador(ClassFile *, JVM *);
void VerificadorOverride(ClassFile *, JVM *);
