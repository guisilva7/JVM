// validador de estruturas
#include "jvm.h"

bool DescritorCampo(uint16_t, constant_pool_info *);
bool DescritorMetodo(constant_pool_info *, uint16_t);
void verifyConstantPool(ClassFile *);
void AcessoFlags(ClassFile *);
void VerBytecode(attribute_info *, ClassFile *);
void ArquivoClassverificador(ClassFile *);
void SuperVerificador(ClassFile *, JVM *);
void VerificadorOverride(ClassFile *, JVM *);
