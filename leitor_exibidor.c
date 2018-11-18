#include "leitor_exibidor.h"
#include "opcodes.h"

#ifdef __linux__
    #define HOWTO "./leitor_exibidor"
#else
    #define HOWTO "leitor_exibidor.exe"
#endif

char* op_codesJVM[];


// retorna 1 byte lido do arquivo de entrada
uint8_t u1Read(FILE * file)
{
    int8_t  to_return = getc(file);
    // if(to_return == EOF)
    // {
    //     puts("[ERRO] erro ao ler um bytes");
    //     exit(EXIT_FAILURE);
    // }
    return  to_return;
}

// retorna 2 bytes lidos do arquivo de entrada
uint16_t u2Read(FILE * file)
{
    int16_t to_return = getc(file);
    // if(to_return == EOF)
    // {
    //     puts("[ERRO] erro ao ler dois bytes");
    //     exit(EXIT_FAILURE);
    // }
    
    uint8_t to_return2 = u1Read(file);
    
    to_return = (to_return << 8) | to_return2;
    return  to_return;
}

// retorna 4 bytes lidos do arquivo de entrada
uint32_t u4Read(FILE * file)
{
    uint32_t    to_return = getc(file);
    if(to_return == EOF)
    {
        puts("[ERRO] erro ao quatro bytes");
        exit(EXIT_FAILURE);
    }

    uint8_t to_return2 = u1Read(file);
    to_return = (to_return << 8) | to_return2; 
    
    to_return2 = u1Read(file);
    to_return =  (to_return << 8) | to_return2;
    
    to_return2 = u1Read(file);
    to_return =  (to_return << 8) | to_return2;
    return  to_return;
}

// retorna o tipo de atributo.
ATTRIBUTE_TYPE getAttributeType(attribute_info * attr, ClassFile * cf)
{
    // attribute_name_index indica indice da CONTANT_Utf8 da contant_pool
    // que indica o tipo do atributo
    uint16_t name_index = attr->attribute_name_index;
    constant_pool_info * constPool = cf->constant_pool + name_index - 1;
    
    // CONSTANT_Utf8
    uint16_t length = constPool->u.Utf8.length;
    uint8_t* bytes = constPool->u.Utf8.bytes;
    
    // tamanho maior que zero
    if(length)
    {
        switch(bytes[0])
        {
            // Code ou ConstantValue
            case 'C':
                if(!strcmp((char *) bytes, "Code")){
                    return  CODE;
                }
                else if(!strcmp((char *) bytes, "ConstantValue")){
                    return  CONSTANT_VALUE;                 
                }
                break;
            // Deprecated
            case 'D':
                if(!strcmp((char *) bytes, "Deprecated")){
                    return  DEPRECATED;
                }
                break;
            // Exceptions
            case 'E':
                if(!strcmp((char *) bytes, "Exceptions")){
                    return  EXCEPTIONS;
                }
                break;
            // InnerClasses
            case 'I':
                if(!strcmp((char *) bytes, "InnerClasses")){
                    return  INNER_CLASSES;
                }
                break;
            // LineNumberTable ou LocalVariableTable
            case 'L':
                if(!strcmp((char *) bytes, "LineNumberTable")){
                    return  LINE_NUMBER_TABLE;
                }
                else if(!strcmp((char *) bytes, "LocalVariableTable")){
                    return  LOCAL_VARIABLE_TABLE;
                }
                break;
            // Synthetic ou SourceFile
            case 'S':
                if(!strcmp((char *) bytes, "Synthetic")){
                    return  SYNTHETIC;
                }
                else if(!strcmp((char *) bytes, "SourceFile")){
                    return  SOURCE_FILE;
                }
                break;
            // nao conhecido
            default:
                return  UNKNOWN;
        }
    }
    return  UNKNOWN;    
}

//-- metodos de obtencao de campos do ClassFile

// faz a leitura do arquivo .class, e retorna um ponteiro para estrutura 
// ClassFile com os campos preenchidos
ClassFile * obtainClassFile(FILE * file)
{
    
    // alocando espaço para estrutura do tipo ClassFile.
    ClassFile * cf = (ClassFile *) malloc(sizeof(ClassFile)); 
    
    // se nao foi possivel alocar
    if (cf == NULL) {
        printf("\n[ERRO] erro ao alocar memoria para ClassFile\n");
        exit(1);
    }
    
    // le 4 bytes como campo magic do ClassFile
    cf->magic = u4Read(file); 
    // caso nao seja CAFEBABE, emite mensagem de erro e sai do programa
    if(cf->magic != 0xCAFEBABE){
        puts("\n[ERRO] (cf->magic) numero magico nao eh CAFEBABE\n");
        free(cf);
        exit(EXIT_FAILURE);
    }
    
    // le 2 bytes como campo minor_version e 2 bytes como campo major_version do ClassFile
    cf->minor_version = u2Read(file);
    cf->major_version = u2Read(file);
    // ESTUDAR - 
    //45 = versao 1.1
    //46 = versao 1.2 
    if((cf->major_version > 46) || (cf->major_version < 45) 
        || (cf->major_version == 46 && cf->minor_version != 0)){
        puts("\n[ERRO] (cf->minor ou cf->major) so suportadas versoes 1.1 e 1.2\n");
        // exit(EXIT_FAILURE);
    }
    
    // obtem constant_pool do ClassFile
    obtainConstantPool(cf, file);
    
    // le 2 bytes como campo access_flags do ClassFile
    cf->access_flags = u2Read(file);      
    
    // le 2 bytes como campo this_class do ClassFile
    cf->this_class = u2Read(file);    

    // le 2 bytes como campo super_class do ClassFile
    cf->super_class = u2Read(file);   
    
    // obtem interfaces do ClassFile
    obtainInterfaces(cf, file);
    
    // obtem fields do ClassFile
    obtainFields(cf, file);
    
    // obtem methods do ClassFile
    obtainMethods(cf, file);
    
    // obtem attributes do ClassFile
    obtainAttributes(NULL, NULL, NULL, cf, file);
    
    // verifica final do arquivo ClassFile
    if(getc(file) != EOF){
        puts("\n[ERRO] mais campos que um ClassFile correto\n");
    }
    return  cf;
}

// le e armazena a constant_pool no ClassFile
void  obtainConstantPool(ClassFile * cf, FILE * file)
{
    // obtem numero de entradas da constant_pool + 1 
    cf->constant_pool_count = u2Read(file); // printf("constant_pool_count = %" PRIu16, cf->constant_pool_count);

    if(cf->constant_pool_count == 0)
    {
        puts("\n[ERRO] constant_pool_count igual a zero\n");
        exit(EXIT_FAILURE);
    }
    // alocando memoria para a constant_pool
    // tem o tamanho de constant_pool_count-1 e seus campos sao do tipo constant_pool_info
    cf->constant_pool = (constant_pool_info *) malloc((cf->constant_pool_count -1)*sizeof(constant_pool_info));
    
    // declara um tipo constant_pool_info * para percorrer a constant_pool
    constant_pool_info * constPool; 
    // itera em todos os registros do constant_pool
    for(constPool = cf->constant_pool; constPool < (cf->constant_pool + cf->constant_pool_count - 1); constPool++)
    {
        // constPool->tag = 0;
        constPool->tag = u1Read(file);  // printf("\ncp_tag = %" PRIu8, constPool->tag);
        
        // dependendo do campo tag do contant_pool_info
        // obtem-se qual o tipo da estrutura CONSTANT_X (info[])
        switch(constPool->tag)
        {         
                // 4.4.1. The CONSTANT_Class_info Structure 
            case CONSTANT_Class: // 7
                constPool->u.Class.name_index = u2Read(file);
                break;
                // 4.4.2. The CONSTANT_Fieldref_info, CONSTANT_Methodref_info, and CONSTANT_InterfaceMethodref_info Structures 
            case CONSTANT_Fieldref: // 9
            case CONSTANT_Methodref: // 10
            case CONSTANT_InterfaceMethodref: // 11
                constPool->u.Ref.name_index = u2Read(file);
                constPool->u.Ref.name_and_type_index = u2Read(file);
                break;
                // 4.4.3. The CONSTANT_String_info Structure 
            case CONSTANT_String: // 8
                constPool->u.String.string_index = u2Read(file);
                break;
                // 4.4.4. The CONSTANT_Integer_info and CONSTANT_Float_info Structures 
            case CONSTANT_Integer: // 3
            case CONSTANT_Float: // 4
                constPool->u.Integer_Float.bytes = u4Read(file);
                break;
                // 4.4.5. The CONSTANT_Long_info and CONSTANT_Double_info Structures 
            case CONSTANT_Long: // 5
            case CONSTANT_Double: // 6
                constPool->u.Long_Double.high_bytes = u4Read(file);
                constPool->u.Long_Double.low_bytes = u4Read(file);
                // pula por ser 64 bits
                constPool++; 
                break;
                // 4.4.6. The CONSTANT_NameAndType_info Structure
            case CONSTANT_NameAndType: // 12
                constPool->u.NameAndType.name_index = u2Read(file);
                constPool->u.NameAndType.descriptor_index = u2Read(file);
                break;
                // 4.4.7. The CONSTANT_Utf8_info Structure 
            case CONSTANT_Utf8: // 1
                constPool->u.Utf8.length = u2Read(file);
                
                // se tamanho maior que zero
                if(constPool->u.Utf8.length)
                {
                    constPool->u.Utf8.bytes = (uint8_t *) malloc((constPool->u.Utf8.length + 1) * sizeof(uint8_t));
                    // obtem cada byte do arquivo de acordo com o tamanho de lenght
                    // ao fim coloca nulo (\0) para ser uma string
                    for(uint16_t i = 0; i < constPool->u.Utf8.length; i++)
                    {
                        constPool->u.Utf8.bytes[i] = u1Read(file);
                    }   
                    constPool->u.Utf8.bytes[constPool->u.Utf8.length] = '\0';
                // se tamanho eh zero
                }else
                {
                    constPool->u.Utf8.bytes = NULL;
                }
                break;
            // nenhum dos casos anteriores eh um erro
            default:
                puts("\n[ERRO] atributo de constant_pool nao valido\n");
                exit(EXIT_FAILURE);
        }   
    }
}

// le e armazena as interfaces no ClassFile
void obtainInterfaces(ClassFile * cf, FILE * file)
{
    // recebe o numero direto de superinterfaces dessa classe ou tipos de interface
    cf->interfaces_count = u2Read(file); 
    // se tamanho maior que zero
    if(cf->interfaces_count)
    {
        // aloca vetor de interfaces de acordo com interfaces_count
        cf->interfaces = (uint16_t *) malloc((cf->interfaces_count)*sizeof(uint16_t));
        
        // para cada registro alocado, obtem dois bytes do arquivo
        for(uint16_t * in = cf->interfaces; in < (cf->interfaces + cf->interfaces_count); in++)
        {
            *in = u2Read(file);
        }
    }
    // se tamanho eh zero
    else{
        cf->interfaces = NULL;
    }
    
}

// le e armazena os fields no ClassFile
void  obtainFields(ClassFile * cf, FILE * file)
{
    // recebe o numero de estruturas field_info para alocar
    cf->fields_count = u2Read(file); 
    // se tamanho maior que zero
    if(cf->fields_count)
    {
        // aloca vetor de field_info de acordo com fields_count
        cf->fields = (field_info *) malloc((cf->fields_count)*sizeof(field_info));

        // para cada registro field_info 
        for(field_info * fd_in = cf->fields; fd_in < (cf->fields + cf->fields_count); fd_in++)
        {
            fd_in->access_flags = u2Read(file);
            fd_in->name_index = u2Read(file);
            fd_in->descriptor_index = u2Read(file);
            // obtem attributes_count e vetor de attributes_info
            obtainAttributes(fd_in, NULL, NULL, cf, file);
        }
    }
    // se tamanho eh zero
    else{
        cf->fields = NULL;
    }
}

// le e armazena os methods no ClassFile
void obtainMethods(ClassFile * cf, FILE * file)
{
    // recebe o numero de estruturas method_info para alocar
    cf->methods_count = u2Read(file);
    // se tamanho maior que zero
    if(cf->methods_count)
    {
        // aloca vetor de method_info de acordo com methods_count
        cf->methods = (method_info *) malloc((cf->methods_count)*sizeof(method_info));

        // para cada registro method_info 
        for(method_info * mt_in = cf->methods; mt_in < (cf->methods + cf->methods_count); mt_in++)
        {
            mt_in->access_flags = u2Read(file);
            mt_in->name_index = u2Read(file);
            mt_in->descriptor_index = u2Read(file);
            // obtem attributes_count e vetor de attributes_info
            obtainAttributes(NULL, mt_in, NULL, cf, file);
        }
    }
    // se tamanho eh zero
    else{
        cf->methods = NULL;
    }
    
}

// le e armazena os attributes no ClassFile
// usada para attributes de field_info, de method_info e de attributes_info
void obtainAttributes(field_info * fd_in, method_info * mt_in, attribute_info * attr_in, ClassFile * cf,FILE * file)
{
    // para percorrer os registros attributes_info
    attribute_info *    attributes;
    uint16_t            attributes_count;
    
    // se for attributes de field_info
    if(fd_in != NULL)
    {
        fd_in->attributes_count = u2Read(file);
        
        // se tamanho maior que zero
        if(fd_in->attributes_count)
        {
            // aloca vetor de attributes_info
            fd_in->attributes = (attribute_info *) malloc(fd_in->attributes_count * sizeof(attribute_info));    
        // se tamanho igual a zero
        }else
        {
            fd_in->attributes = NULL;
        }
        attributes_count = fd_in->attributes_count;
        attributes = fd_in->attributes;
    }
    // se for attributes de methos_info
    else if(mt_in != NULL)
    {
        mt_in->attributes_count = u2Read(file);
        
        // se tamanho maior que zero
        if(mt_in->attributes_count)
        {
            // aloca vetor de attributes_info
            mt_in->attributes = (attribute_info *) malloc(mt_in->attributes_count * sizeof(attribute_info));    
        // se tamanho igual a zero
        }else
        {
            mt_in->attributes = NULL;
        }       
        attributes_count = mt_in->attributes_count;
        attributes = mt_in->attributes;
    }
    // se for attributes de attributes [CODE]
    else if(attr_in != NULL)
    {
        attr_in->u.Code.attributes_count = u2Read(file);

        // se tamanho maior que zero
        if(attr_in->u.Code.attributes_count)
        {
            // aloca vetor de attributes_info
            attr_in->u.Code.attributes = (attribute_info *) malloc(attr_in->u.Code.attributes_count * sizeof(attribute_info));
        // se tamanho igual a zero
        }else
        {
            attr_in->u.Code.attributes = NULL;
        }
        attributes_count = attr_in->u.Code.attributes_count;
        attributes = attr_in->u.Code.attributes;
    }
    // se for attributes do ClassFile
    else if(cf != NULL)
    {
        cf->attributes_count = u2Read(file);

        // se tamanho maior que zero
        if(cf->attributes_count)
        {
            // aloca vetor de attributes_info
            cf->attributes = (attribute_info *) malloc(cf->attributes_count * sizeof(attribute_info));
        // se tamanho igual a zero
        }else
        {
            cf->attributes = NULL;
        }
        attributes_count = cf->attributes_count;
        attributes = cf->attributes;
    }
    // se nenhum dos casos anteriores
    else{
        puts("[ERRO] nao foi possivel obter os attributes");
        exit(EXIT_FAILURE);
    }
    
    // percorre os registros attribute_info de acordo com a quantidade attributes_count e o 
    // vetor de atributos attributes
    for(attribute_info * attr = attributes; attr < (attributes + attributes_count); attr++)
    {
        attr->attribute_name_index = u2Read(file);
        attr->attribute_length = u4Read(file);
        
        // obtem tipo de atributo de acordo com a contant_pool
        ATTRIBUTE_TYPE attrTypeResult = getAttributeType(attr, cf);
        
        // dependendo do tipo fo atributo
        switch(attrTypeResult)
        {
            // 4.7.2. The ConstantValue Attribute 
            case CONSTANT_VALUE: // 0
                attr->u.ConstantValue.constantvalue_index = u2Read(file);
                break;
            // 4.7.3. The Code Attribute 
            case CODE: // 1
                 attr->u.Code.max_stack = u2Read(file);
                    attr->u.Code.max_locals = u2Read(file);
                    attr->u.Code.code_length = u4Read(file);
                    if(attr->u.Code.code_length){
                        attr->u.Code.code = (uint8_t *) malloc(attr->u.Code.code_length * sizeof(uint8_t));
                        for(uint8_t * cd = attr->u.Code.code; cd < (attr->u.Code.code + attr->u.Code.code_length); cd++)
                            *cd = u1Read(file);
                    }
                    
                    
                    attr->u.Code.exception_table_length = u2Read(file);
                    if(attr->u.Code.exception_table_length){
                        attr->u.Code.exception_table =
                        (exception_table_type *) malloc(attr->u.Code.exception_table_length * sizeof(exception_table_type));
                        
                        for(exception_table_type * ex_tb = attr->u.Code.exception_table;
                            ex_tb < (attr->u.Code.exception_table + attr->u.Code.exception_table_length); ex_tb++){
                            ex_tb->start_pc = u2Read(file);
                            ex_tb->end_pc = u2Read(file);
                            ex_tb->handler_pc = u2Read(file);   
                            ex_tb->catch_type = u2Read(file);                   
                        }
                    }
                    obtainAttributes(NULL, NULL, attr, cf, file);
                    break;
            // 4.7.15. The Deprecated Attribute 
            case DEPRECATED: // 2
                // nada
                break;
            // 4.7.5. The Exceptions Attribute 
            case EXCEPTIONS: // 3
                attr->u.Exceptions.number_of_exceptions = u2Read(file);
                    attr->u.Exceptions.exception_index_table = (uint16_t *) malloc(attr->u.Exceptions.number_of_exceptions * sizeof(uint16_t));
                    for(uint16_t * ex_in_tb = attr->u.Exceptions.exception_index_table; 
                        ex_in_tb < (attr->u.Exceptions.exception_index_table + attr->u.Exceptions.number_of_exceptions); ex_in_tb++)
                        * ex_in_tb = u2Read(file);  
                break;
            // 4.7.6. The InnerClasses Attribute 
            case INNER_CLASSES: // 4
                attr->u.InnerClasses.number_of_classes = u2Read(file);
                    attr->u.InnerClasses.classes =
                    (classes_type *) malloc(attr->u.InnerClasses.number_of_classes * sizeof(classes_type));
                    for(classes_type * cl = attr->u.InnerClasses.classes;
                        cl < (attr->u.InnerClasses.classes + attr->u.InnerClasses.number_of_classes); cl++){
                        cl->inner_class_info_index = u2Read(file);
                        cl->outer_class_info_index = u2Read(file);
                        cl->inner_name_index = u2Read(file);
                        cl->inner_class_access_flags = u2Read(file);
                    }
                break;
            // 4.7.12. The LineNumberTable Attribute 
            case LINE_NUMBER_TABLE: // 5
                attr->u.LineNumberTable.line_number_table_length = u2Read(file);
                    if(attr->u.LineNumberTable.line_number_table_length){
                        attr->u.LineNumberTable.line_number_table = (line_number_table_type *)
                        malloc(attr->u.LineNumberTable.line_number_table_length * sizeof(line_number_table_type));
                        
                        for(line_number_table_type * ln_tb = attr->u.LineNumberTable.line_number_table; 
                            ln_tb < (attr->u.LineNumberTable.line_number_table + attr->u.LineNumberTable.line_number_table_length); ln_tb++){   
                            ln_tb->start_pc = u2Read(file);
                            ln_tb->line_number = u2Read(file);
                        }           
                    }
                break;
            // 4.7.13. The LocalVariableTable Attribute 
            case LOCAL_VARIABLE_TABLE: // 6
                attr->u.LocalVariableTable.local_variable_table_length = u2Read(file);
                    if(attr->u.LocalVariableTable.local_variable_table_length){
                        attr->u.LocalVariableTable.local_variable_table = (local_variable_table_type *)
                        malloc(attr->u.LocalVariableTable.local_variable_table_length * sizeof(local_variable_table_type));
                        
                        for(local_variable_table_type * lv_tb = attr->u.LocalVariableTable.local_variable_table; 
                            lv_tb < (attr->u.LocalVariableTable.local_variable_table +
                                     attr->u.LocalVariableTable.local_variable_table_length); lv_tb++){
                                lv_tb->start_pc = u2Read(file);
                                lv_tb->length = u2Read(file);
                                lv_tb->name_index = u2Read(file);
                                lv_tb->descriptor_index = u2Read(file);
                                lv_tb->index = u2Read(file);
                            }
                    }
                break;
            // 4.7.8. The Synthetic Attribute 
            case SYNTHETIC: // 7
                // nada
                break;
            // 4.7.10. The SourceFile Attribute 
            case SOURCE_FILE: // 8
                attr->u.SourceFile.sourcefile_index = u2Read(file);
                break;
            default: // UNKNOWN
                ;// nenhum caso conhecido
        }
    }
}

//-- metodos para mostrar no arquivo de saida os campos do ClassFile

// mostra ClassFile no arquivo de saida
void showClassFile(ClassFile * cf, FILE * saida)
{
    fprintf(saida, "ByteCode viewer\n");
    // inclui Magic, Version, Access Flags, this Class, super Class, Attributes
    showGeneralInfo(cf, saida);
    showConstantPool(cf, saida);
    showInterfaces(cf, saida);
    showFields(cf, saida);
    showMethods(cf, saida);
    showAttributes(NULL, NULL, NULL, cf, saida);
}

void showGeneralInfo(ClassFile * cf,  FILE * saida)
{
    float versao;
    constant_pool_info * constPool;
    
    // mostrar Informação Geral
    fprintf(saida,"\n#General Information\n");
    fprintf(saida, "Magic:\t\t\t\t\t\t0x%" PRIX32 "\n", cf->magic);               // hexadecimal printf format for uint32_t 
    fprintf(saida, "Minor version:\t\t\t\t%" PRId16 "\n", cf->minor_version); // decimal printf format for int16_t 
    
    switch(cf->major_version)
    {
        case 45:
            versao = 1.1;
            break;
        case 46:
            versao = 1.2;
            break;
        case 47:
            versao = 1.3;
            break;
        case 48:
            versao = 1.4;
            break;
        case 49:
            versao = 1.5;
            break;
        case 50:
            versao = 1.6;
            break;
        case 51:
            versao = 1.7;
            break;
        case 52:
            versao = 1.8;
            break;
        default: // signfica que deu erro...
            versao = 0.0;
    }

    fprintf(saida, "Major version:\t\t\t\t%" PRIu16 "\t[%.1f]\n", cf->major_version, versao); // decimal printf format for uint16_t 
    fprintf(saida, "Constant pool count:\t\t%" PRIu16 "\n", cf->constant_pool_count);
    fprintf(saida, "Access flags:\t\t\t\t0x%.4" PRIX16 "\t[", cf->access_flags);
    
    uint16_t access_flags = cf->access_flags;
    
    // o access_flags eh um campo de 16 bits
    //em que cada bit significa uma flag
    //se for 1 significa que a tal flag esta setada
    //cc nao esta
    //dessa forma, access_flags tem sua representacao em decimal
    //e subtraindo o valor decimal de cada flag, podemos testar 
    //os proximos bits menos signifcativos
    if(access_flags >= ACC_ABSTRACT)
    {
        fprintf(saida, "abstract ");
        access_flags -= ACC_ABSTRACT;
    }
    if(access_flags >= ACC_INTERFACE){
        fprintf(saida, "interface ");
        access_flags -= ACC_INTERFACE;
    }
    if(access_flags >= ACC_SUPER){
        fprintf(saida, "super ");
        access_flags -= ACC_SUPER;
    }
    
    if(access_flags >= ACC_FINAL){
        fprintf(saida, "final ");
        access_flags -= ACC_FINAL;
    }
    if(access_flags >= ACC_PUBLIC){
        fprintf(saida, "public ");
        access_flags -= ACC_PUBLIC;
    }
    fprintf(saida,"]\n");
    
    // obtainlocale (LC_ALL, "" );
    
    fprintf(saida, "This class:\t\t\t\t\tcp_info #%" PRIu16, cf->this_class);
    
    if(cf->constant_pool[cf->this_class-1].tag != CONSTANT_Class)
    {
        puts("\n[ERRO] this_class nao aponta para uma CONSTANT_Class_info\n");
        exit(EXIT_FAILURE);
    }else{
        uint16_t name_index;
        // o name_index referente ao cp_info CONSTANT_Class_info indexado pelo this_class
        // contem o indice da CONSTANT_Utf8 que armazena o nome da classe
        name_index = cf->constant_pool[cf->this_class-1].u.Class.name_index;
        constPool = cf->constant_pool + name_index - 1;
        if(constPool->tag != CONSTANT_Utf8)
        {
            puts("\n[ERRO] this_class nao referencia um name_index valido\n");
            exit(EXIT_FAILURE);
        }
        else{
            fprintf(saida, "\t<");
            printConstUtf8(constPool, saida);
            fprintf(saida, ">\n");
        }
    }
    
    // se for zero
    if(!cf->super_class)
    {
        fprintf(saida, "Super class:\t\t\t\tNone\n");
    }
    // maior que zero
    else{
        if(cf->constant_pool[cf->super_class-1].tag != CONSTANT_Class)
        {
            puts("\n[ERRO] super_class nao aponta para uma CONSTANT_Class_info\n");
            exit(EXIT_FAILURE);
        }else
        {
            fprintf(saida, "Super class:\t\t\t\tcp_info #%" PRIu16 , cf->super_class);
            uint16_t name_index;
            name_index = cf->constant_pool[cf->super_class-1].u.Class.name_index;
            constPool = cf->constant_pool + name_index - 1;
            if(constPool->tag != CONSTANT_Utf8)
            {
                puts("\n[ERRO] super_class nao referencia um name_index valido\n");
                exit(EXIT_FAILURE);
            }else
            {
                fprintf(saida, "\t<");
                printConstUtf8(constPool, saida);
                fprintf(saida, ">\n");
            }
        }
    }
    
    fprintf(saida, "Interfaces count:\t\t\t%" PRIu16 "\n", cf->interfaces_count);
    fprintf(saida, "Fields count:\t\t\t\t%" PRIu16 "\n", cf->fields_count);
    fprintf(saida, "Methods count:\t\t\t\t%" PRIu16 "\n", cf->methods_count);
    fprintf(saida, "Attributes count:\t\t\t%" PRIu16 "\n", cf->attributes_count);
}

// imprime no arquivo de saida a constant_pool do ClassFile
void showConstantPool(ClassFile * cf, FILE * saida)
{
    int64_t bits64;
    int32_t bits32;
    fprintf(saida,"\n#Constant Pool\n");
    constant_pool_info * constPool;
    uint16_t i;
    // itera em todos os registros contant_info da contant_pool
    for(constPool = cf->constant_pool, i = 1; constPool < (cf->constant_pool + cf->constant_pool_count - 1); constPool++, i++)
    {
        // dependendo da tag (tipo CONTANT_), mostra o conteudo
        switch(constPool->tag)
        {
            case CONSTANT_Class:
                fprintf(saida, "[%" PRIu16 "] CONSTANT_Class_info\n", i);
                fprintf(saida, "\tClass name:\t\t\t\tcp_info #%" PRIu16 "\t<", constPool->u.Class.name_index);
                printConstantClass(cf,constPool,saida);
                fprintf(saida, ">\n");
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
                if(constPool->tag == CONSTANT_Fieldref)
                    fprintf(saida, "[%" PRIu16 "] CONSTANT_Fieldref_info\n", i);
                else if(constPool->tag == CONSTANT_Methodref)
                    fprintf(saida, "[%" PRIu16 "] CONSTANT_Methodref_info\n", i);
                else if(constPool->tag == CONSTANT_InterfaceMethodref)
                    fprintf(saida, "[%" PRIu16 "] CONSTANT_InterfaceMethodref_info\n", i);
                
                // This constant_pool entry indicates the name and descriptor of the field or method.
                // In a CONSTANT_Fieldref_info, the indicated descriptor must be a field descriptor (§4.3.2). 
                // Otherwise, the indicated descriptor must be a method descriptor (§4.3.3).
                // If the name of the method of a CONSTANT_Methodref_info structure begins with a '<' ('\u003c'), 
                // then the name must be the special name <init>, representing an instance initialization method (§2.9). 
                // The return type of such a method must be void. 
                fprintf(saida, "\tClass name:\t\t\t\tcp_info #%" PRIu16 "\t<", constPool->u.Ref.name_index);
                printConstantRef(cf,constPool,'c',saida); // Class name
                fprintf(saida, ">\n\tName and type:\t\t\tcp_info #%" PRIu16 "\t<", constPool->u.Ref.name_and_type_index);
                printConstantRef(cf,constPool,'n',saida); // Name and type
                fprintf(saida, ">\n");
                break;
            case CONSTANT_String:
                fprintf(saida, "[%" PRIu16 "] CONSTANT_String_info\n", i);
                fprintf(saida, "\tString:\t\t\t\tcp_info #%" PRIu16 "\t<", constPool->u.String.string_index);
                printConstantString(cf,constPool,saida);
                fprintf(saida, ">\n");
                break;
            case CONSTANT_Integer:
                fprintf(saida, "[%" PRIu16 "] CONSTANT_Integer_info\n", i);
                fprintf(saida, "\tBytes:\t\t\t0x%.8" PRIx32, constPool->u.Integer_Float.bytes);
                fprintf(saida, "\n\tInteger:\t\t\t%" PRId32 "\n", constPool->u.Integer_Float.bytes);
                break;
            case CONSTANT_Float:
                // int s = ((bits >> 31) == 0) ? 1 : -1;
                // int e = ((bits >> 23) & 0xff);
                // int m = (e == 0) ?
                //           (bits & 0x7fffff) << 1 :
                //           (bits & 0x7fffff) | 0x800000;

                fprintf(saida, "[%" PRIu16 "] CONSTANT_Float_info\n", i);
                fprintf(saida, "\tBytes:\t\t\t0x%.8" PRIx32, constPool->u.Integer_Float.bytes);
                bits32 = (int32_t) constPool->u.Integer_Float.bytes;
                
                switch(bits32)
                {
                    // If bits is 0x7f800000, the float value will be positive infinity. 
                    case 0x7f800000:
                        fprintf(saida, "\n\tFloat:\t\t+infinity\n");
                        break;
                    // If bits is 0xff800000, the float value will be negative infinity. 
                    case 0xff800000:
                        fprintf(saida, "\n\tFloat:\t\t-infinity\n");
                        break;
                    default:
                        // If bits is in the range 0x7f800001 through 0x7fffffff or in the range 0xff800001 
                        // through 0xffffffff, the float value will be NaN.  
                        if((bits32 >= 0x7f800001 && bits32 <= 0x7fffffff) ||
                           (bits32 >= 0xff800001 && bits32 <= 0xffffffff )){
                            fprintf(saida, "\n\tFloat:\t\tNaN\n");
                        }
                        // In all other cases, let s, e, and m be three values that might be computed from bits: 
                        else{
                            int32_t s = ((bits32 >> 31) == 0) ? 1 : -1;
                            int32_t e = ((bits32 >> 23) & 0xff);
                            int32_t m = (e == 0) ?
                                    (bits32 & 0x7fffff) << 1 :
                                    (bits32 & 0x7fffff) | 0x800000;
                            // Then the float value equals the result of the mathematical expression s · m · 2e-150. 
                            fprintf(saida, "\n\tFloat:\t\t%f\n", (double) s*m*pow(2, (e-150)));
                        }
                }
                break;
            case CONSTANT_Long:
                // ((long) high_bytes << 32) + low_bytes
                
                fprintf(saida, "[%" PRIu16 "] CONSTANT_Long_info\n", i);
                fprintf(saida, "\tHigh bytes:\t\t\t\t0x%.8" PRIx32, constPool->u.Long_Double.high_bytes);
                fprintf(saida, "\n\tLow bytes:\t\t\t\t0x%.8" PRIx32, constPool->u.Long_Double.low_bytes);
                bits64 = (((int64_t) constPool->u.Long_Double.high_bytes) << 32) + constPool->u.Long_Double.low_bytes;
                fprintf(saida, "\n\tLong:\t\t\t\t\t%" PRId64 "\n", bits64);
                fprintf(saida, "[%" PRIu16 "]\t(large numeric continued)\n", ++i);
                // pula por ser 64 bits
                constPool++;
                break;
            case CONSTANT_Double:
                // ((long) high_bytes << 32) + low_bytes
                // int s = ((bits >> 63) == 0) ? 1 : -1;
                // int e = (int)((bits >> 52) & 0x7ffL);
                // long m = (e == 0) ?
                //            (bits & 0xfffffffffffffL) << 1 :
                //            (bits & 0xfffffffffffffL) | 0x10000000000000L;

                fprintf(saida, "[%" PRIu16 "] CONSTANT_Double_info\n", i);
                fprintf(saida, "\tHigh bytes:\t\t0x%.8" PRIx32, constPool->u.Long_Double.high_bytes);
                fprintf(saida, "\n\tLow bytes:\t\t0x%.8" PRIx32, constPool->u.Long_Double.low_bytes);
                bits64 = (((int64_t) constPool->u.Long_Double.high_bytes) << 32) + constPool->u.Long_Double.low_bytes;
                
                switch(bits64)
                {
                    // If bits is 0x7ff0000000000000L, the double value will be positive infinity. 
                    case 0x7ff0000000000000L:
                        fprintf(saida, "\n\tDouble:\t\t+infinity\n");
                        break;
                    // If bits is 0xfff0000000000000L, the double value will be negative infinity. 
                    case 0xfff0000000000000L:
                        fprintf(saida, "\n\tDouble:\t\t-infinity\n");
                        break;
                    default:
                        // If bits is in the range 0x7ff0000000000001L through 0x7fffffffffffffffL or in the range 0xfff0000000000001L 
                        // through 0xffffffffffffffffL, the double value will be NaN. 
                        if((bits64 >= 0x7ff0000000000001L && bits64 <= 0x7ffffffffffffL) ||
                           (bits64 >= 0xfff0000000000001L && bits64 <= 0xffffffffffffffffL )){
                            fprintf(saida, "\n\tDouble:\t\tNaN\n");
                        }
                        // In all other cases, let s, e, and m be three values that might be computed from bits: 
                        else{
                            int32_t s = ((bits64 >> 63) == 0) ? 1 : -1;
                            int32_t e = ((bits64 >> 52) & 0x7ffL);
                            int64_t m = (e == 0) ?
                                    (bits64 & 0xfffffffffffffL) << 1 :
                                    (bits64 & 0xfffffffffffffL) | 0x10000000000000L;
                            // Then the floating-point value equals the double value of the mathematical expression s · m · 2e-1075
                            fprintf(saida, "\n\tDouble:\t\t%f\n", (double) s*m*pow(2, (e-1075)));
                        }
                }
                fprintf(saida, "[%" PRIu16 "]\t(large numeric continued)\n", ++i);
                // pula por ser 64 bits
                constPool++;
                break;
            case CONSTANT_NameAndType:
                fprintf(saida, "[%" PRIu16 "] CONSTANT_NameAndType_info\n", i);
                fprintf(saida, "\tName:\t\t\t\t\tcp_info #%" PRIu16 "\t<", constPool->u.NameAndType.name_index);
                // caso Name
                printConstantNameAndType(cf,constPool,'n',saida);
                fprintf(saida, ">\n\tDescriptor:\t\t\t\tcp_info #%" PRIu16 "\t<", constPool->u.NameAndType.descriptor_index);
                // caso Descritor
                printConstantNameAndType(cf,constPool,'d',saida);
                fprintf(saida, ">\n");
                break;
            case CONSTANT_Utf8:
                fprintf(saida, "[%" PRIu16 "] CONSTANT_Utf8_info\n", i);
                fprintf(saida, "\tLength of byte array:\t%" PRIu16, constPool->u.Utf8.length);
                fprintf(saida, "\n\tLength of string:\t\t%" PRIu16, constPool->u.Utf8.length);
                fprintf(saida, "\n\tString:\t\t\t\t\t");
                printConstUtf8(constPool, saida);
                fprintf(saida, "\n");
                break;
        }
    }
}

// imprime interfaces do ClassFile
void showInterfaces(ClassFile * cf, FILE * saida)
{
    
    fprintf(saida,"\n#Interfaces\n");
    
    for(uint16_t i = 0; i < cf->interfaces_count; i++)
    {
        fprintf(saida, "Interface %" PRIu16, i);
        
        fprintf(saida,"\tcp_info #%" PRIu16, cf->interfaces[i]);
        
        // se nao for CONSTANT_Class
        if(cf->constant_pool[cf->interfaces[i]-1].tag != CONSTANT_Class)
        {
            puts("\n[ERRO] interface nao referencia uma CONSTANTE_Class_info\n");
            exit(EXIT_FAILURE);
        }else
        {
            uint16_t name_index;
            constant_pool_info * constPool;
            
            name_index = cf->constant_pool[cf->interfaces[i]-1].u.Class.name_index;
            constPool = cf->constant_pool + name_index - 1;
            // se nao for CONSTANT_Utf8
            if(constPool->tag != CONSTANT_Utf8)
            {
                puts("\n[ERRO] interface nao referencia um class_name_index valido\n");
                exit(EXIT_FAILURE);
            }else
            {
                fprintf(saida, "\t<");
                printConstUtf8(constPool, saida);
                fprintf(saida, ">\n\n");
            }
        }
    }
}

// showFields
// imprime fields do ClassFile
void showFields(ClassFile * cf, FILE * saida)
{
    fprintf(saida,"\n#Fields\n");
    // imprime numero de fields
    fprintf(saida,"Member count:\t\t\t\t%" PRIu16 "\n", cf->fields_count);
    // para cada field
    for(uint16_t i = 0; i < cf->fields_count; i++)
    {
        fprintf(saida,"\n");
        uint16_t name_index = (cf->fields+i)->name_index;
        constant_pool_info * constPool = cf->constant_pool + name_index - 1;
        // se nao for CONSTANT_Utf8
        if(constPool->tag != CONSTANT_Utf8)
        {
            puts("\n[ERRO] field nao referencia um name_index valido.\n");
            exit(EXIT_FAILURE);
        }
        //printa numero do field
        fprintf(saida, "[%" PRIu16 "]\t", i);
        printConstUtf8(constPool, saida);
        fprintf(saida, "\n");
        //printa o name entre <>
        fprintf(saida,"\tName:\t\tcp_info #%" PRIu16, (cf->fields+i)->name_index);
        fprintf(saida, "\t<");
        printConstUtf8(constPool, saida);
        fprintf(saida, ">\n");
        
        uint16_t    descriptor_index = (cf->fields+i)->descriptor_index;
        constPool = cf->constant_pool + descriptor_index - 1;
        // se nao for CONSTANT_Utf8
        if(constPool->tag != CONSTANT_Utf8)
        {
            puts("\nERRO: field nao referencia um descriptor_index valido.\n");
            exit(EXIT_FAILURE);
        }
        // printa o descriptor
        fprintf(saida,"\tDescriptor:\t\tcp_info #%" PRIu16, (cf->fields+i)->descriptor_index);
        fprintf(saida, "\t<");
        printConstUtf8(constPool, saida);
        fprintf(saida, ">\n");
        
        // printa acess_flags em hexadecimal 
        fprintf(saida,"\tAccess Flags:\t\t0x%.4" PRIx16 "\t\t[", (cf->fields+i)->access_flags);
        uint16_t access_flags = (cf->fields+i)->access_flags;
        // o access_flags eh um campo de 16 bits
        //em que cada bit significa uma flag
        //se for 1 significa que a tal flag esta setada
        //cc nao esta
        //dessa forma, access_flags tem sua representacao em decimal
        //e subtraindo o valor decimal de cada flag, podemos testar 
        //os proximos bits menos significativos

        if(access_flags >= ACC_TRANSIENT)
        {
            fprintf(saida, "transient ");
            access_flags -= ACC_TRANSIENT;
        }
        if(access_flags >= ACC_VOLATILE)
        {
            fprintf(saida, "volatile ");
            access_flags -= ACC_VOLATILE;
        }
        if(access_flags >= ACC_FINAL)
        {
            fprintf(saida, "final ");
            access_flags -= ACC_FINAL;
        }
        if(access_flags >= ACC_STATIC)
        {
            fprintf(saida, "static ");
            access_flags -= ACC_STATIC;
        }
        if(access_flags >= ACC_PROTECTED)
        {
            fprintf(saida, "protected ");
            access_flags -= ACC_PROTECTED;
        }
        if(access_flags >= ACC_PRIVATE)
        {
            fprintf(saida, "private ");
            access_flags -= ACC_PRIVATE;
        }
        if(access_flags >= ACC_PUBLIC)
        {
            fprintf(saida, "public");
            access_flags -= ACC_PUBLIC;
        }
        fprintf(saida,"]\n");
        
        fprintf(saida,"\n\tField's attributes count:\t%" PRIu16 "\n", (cf->fields+i)->attributes_count);
        field_info * fd_in = (cf->fields+i);
        showAttributes(fd_in, NULL, NULL, cf, saida);
    }
    fprintf(saida, "\n");
}

// imprime methods do ClassFile
void showMethods(ClassFile * cf, FILE * saida)
{
    fprintf(saida,"\n#Methods\n");
    // imprime numero de methods
    fprintf(saida,"Member count:\t\t\t\t%" PRIu16 "\n", cf->methods_count);
    // para cada method
    for(uint16_t i = 0; i < cf->methods_count; i++)
    {
        fprintf(saida,"\n");
        uint16_t name_index = (cf->methods+i)->name_index;
        // pega o indice
        constant_pool_info * constPool = cf->constant_pool + name_index - 1;
        // se nao for CONSTANT_Utf8
        if(constPool->tag != CONSTANT_Utf8)
        {
            puts("\n[ERRO] method nao possui um name_index valido.\n");
            exit(EXIT_FAILURE);
        }
        fprintf(saida, "[%" PRIu16 "]\t", i);
        printConstUtf8(constPool, saida);
        fprintf(saida, "\n");
        // printa o name
        fprintf(saida,"Name:\t\t\t\t\t\tcp_info #%" PRIu16, (cf->methods+i)->name_index);
        fprintf(saida, "\t<");
        printConstUtf8(constPool, saida);
        fprintf(saida, ">\n");
        // printa o descriptor
        fprintf(saida,"Descriptor:\t\t\t\t\tcp_info #%" PRIu16, (cf->methods+i)->descriptor_index);
        uint16_t descriptor_index = (cf->methods+i)->descriptor_index;
        constPool = cf->constant_pool + descriptor_index - 1;
        // se nao for CONSTANT_Utf8
        if(constPool->tag != CONSTANT_Utf8)
        {
            puts("\n[ERRO] method nao possui um descriptor_index valido.\n");
            exit(EXIT_FAILURE);
        }else // printa CONSTANT_Utf8
        {
            fprintf(saida, "\t<");
            printConstUtf8(constPool, saida);
            fprintf(saida, ">\n");
        }
        // printa access_flags
        fprintf(saida,"Access flags:\t\t\t\t0x%.4" PRIx16 "\t\t[", (cf->methods+i)->access_flags);
        uint16_t access_flags = (cf->methods+i)->access_flags;
        // o access_flags eh um campo de 16 bits
        //em que cada bit significa uma flag
        //se for 1 significa que a tal flag esta setada
        //cc nao esta
        //dessa forma, access_flags tem sua representacao em decimal
        //e subtraindo o valor decimal de cada flag, podemos testar 
        //os proximos bits menos significativos

        if(access_flags >= ACC_STRICT)
        {
            fprintf(saida, "strict ");
            access_flags -= ACC_STRICT;
        }
        if(access_flags >= ACC_ABSTRACT)
        {
            fprintf(saida, "abstract ");
            access_flags -= ACC_ABSTRACT;
        }
        if(access_flags >= ACC_NATIVE)
        {
            fprintf(saida, "native ");
            access_flags -= ACC_NATIVE;
        }
        if(access_flags >= ACC_VARARGS)
        {
            fprintf(saida, "varargs ");
            access_flags -= ACC_VARARGS;
        }
        if(access_flags >= ACC_BRIDGE)
        {
            fprintf(saida, "bridge ");
            access_flags -= ACC_BRIDGE;
        }
        if(access_flags >= ACC_SYNCHRONIZED)
        {
            fprintf(saida, "synchronized ");
            access_flags -= ACC_SYNCHRONIZED;
        }
        if(access_flags >= ACC_FINAL)
        {
            fprintf(saida, "final ");
            access_flags -= ACC_FINAL;
        }
        if(access_flags >= ACC_STATIC)
        {
            fprintf(saida, "static ");
            access_flags -= ACC_STATIC;
        }
        if(access_flags >= ACC_PROTECTED)
        {
            fprintf(saida, "protected ");
            access_flags -= ACC_PROTECTED;
        }
        if(access_flags >= ACC_PRIVATE)
        {
            fprintf(saida, "private ");
            access_flags -= ACC_PRIVATE;
        }
        if(access_flags >= ACC_PUBLIC)
        {
            fprintf(saida, "public");
            access_flags -= ACC_PUBLIC;
        }
        fprintf(saida,"]\n");
        // printa os attributes
        fprintf(saida,"\nMethods's attr count:\t\t%" PRIu16 "\n", (cf->methods+i)->attributes_count);
        method_info *   mt_in = (cf->methods+i);
        showAttributes(NULL, mt_in, NULL, cf, saida);
    }
}

// mostra attributes do ClassFile, fields, methods, attributes(dentro do Code    )
// bastando apenas colocar NULL nos argumentos que nao serao utilizados
void showAttributes(field_info * fd_in, method_info * mt_in, attribute_info * attr_in, ClassFile * cf, FILE * saida){
    uint16_t attributes_count;
    attribute_info * attributes;
    int attribute_tab;
    
    // se for attributes do fields
    if(fd_in != NULL)
    {
        fprintf(saida,"\n\tFields Attributes\n");
        attributes_count = fd_in->attributes_count;
        attributes = fd_in->attributes;
        attribute_tab = METHOD_FIELD;
    }
    // se for attributes do method
    else if(mt_in != NULL){
        fprintf(saida,"\n\tMethods Attributes\n");
        attributes_count = mt_in->attributes_count;
        attributes = mt_in->attributes;
        attribute_tab = METHOD_FIELD;
    }
    // se for attributes do attributes code
    else if(attr_in != NULL){
        fprintf(saida,"\n\tAttribute's Code Attributes\n");
        attributes_count = attr_in->u.Code.attributes_count;
        attributes = attr_in->u.Code.attributes;
        attribute_tab = ATTRIBUTE;
    }
    // se for attributes do Classfile
    else if (cf != NULL)
    {
        fprintf(saida,"\n#Attributes\n");
        fprintf(saida, "\nClass Attributes Count:\t\t\t%" PRIu16 "\n", cf->attributes_count);
        attributes_count = cf->attributes_count;
        attributes = cf->attributes;
        attribute_tab = CLASS;
    }
    
    uint16_t name_index;
    constant_pool_info* constPool;
    attribute_info* attributes_aux = attributes;
    // para cada attribute
    for(uint16_t i = 0; i < attributes_count; i++){
        name_index = attributes_aux->attribute_name_index;
        constPool = cf->constant_pool + name_index - 1;
        // se nao for CONSTANT_Utf8
        if(constPool->tag != CONSTANT_Utf8)
        {
            puts("\n[ERRO] atributo nao referencia um name_index valido.\n");
            exit(EXIT_FAILURE);
        }  

        switch(attribute_tab)
        {
                case    CLASS:
                    fprintf(saida,"\n");
                    fprintf(saida, "[%" PRIu16 "]\t", i);
                    printConstUtf8(constPool, saida);
                    fprintf(saida, "\n");
                    
                    fprintf(saida, "Generic info:\n");
                    fprintf(saida, "\tAttribute name index:\tcp_info #%" PRIu16, attributes_aux->attribute_name_index);
                    fprintf(saida, "\t");
                    printConstUtf8(constPool, saida);
                    fprintf(saida, "\n");
                    
                    fprintf(saida, "\tAttribute lenght:\t%" PRIu16 "\n", attributes_aux->attribute_length);
                    fprintf(saida, "Specific info:\n");
                    break;
                case    METHOD_FIELD:
                    fprintf(saida,"\n");
                    fprintf(saida, "[%" PRIu16 "]", i);
                    fprintf(saida, "\t");
                    printConstUtf8(constPool, saida);
                    fprintf(saida, "\n");
                    
                    fprintf(saida, "\tGeneric info:\n");
                    fprintf(saida, "\t\tAttribute name index:\tcp_info #%" PRIu16, attributes_aux->attribute_name_index);
                    fprintf(saida, "\t");
                    printConstUtf8(constPool, saida);
                    fprintf(saida, "\n");
                    
                    fprintf(saida, "\t\tAttribute lenght:\t%" PRIu16 "\n", attributes_aux->attribute_length);
                    fprintf(saida, "\tSpecific info:\n");
                    break;
                case    ATTRIBUTE:
                    fprintf(saida,"\n");
                    fprintf(saida, "\t[%" PRIu16 "]", i);
                    fprintf(saida, "\t");
                    printConstUtf8(constPool, saida);
                    fprintf(saida, "\n");
                    
                    fprintf(saida, "\t\tGeneric info:\n");
                    fprintf(saida, "\t\t\tAttribute name index:\tcp_info #%" PRIu16, attributes_aux->attribute_name_index);
                    fprintf(saida, "\t");
                    printConstUtf8(constPool, saida);
                    fprintf(saida, "\n");
                    
                    fprintf(saida, "\t\t\tAttribute lenght:\t%" PRIu16 "\n", attributes_aux->attribute_length);
                    fprintf(saida, "\t\tSpecific info:\n");
                    break;
            }

        constant_pool_info* cp_a0;
        int64_t bits64;
        int32_t bits32;
        int32_t branch;
        uint16_t constantvalue_index;
        switch(getAttributeType(attributes_aux, cf))
        {
            // se for CONSTANT_VALUE
            case CONSTANT_VALUE:
                ;
                constantvalue_index = attributes_aux->u.ConstantValue.constantvalue_index;
                fprintf(saida, "\t\tCONSTANTvalue index:\tcp_info #%" PRIu16, constantvalue_index);
                cp_a0 = cf->constant_pool + constantvalue_index - 1;
                switch(cp_a0->tag)
                {
                    case CONSTANT_Integer:
                        fprintf(saida, "\t<%" PRId32 ">\n", cp_a0->u.Integer_Float.bytes);
                        break;
                    case CONSTANT_Long:
                    // The unsigned high_bytes and low_bytes items of the CONSTANT_Long_info structure together represent the value of the long constant 
                        bits64 = (((int64_t) cp_a0->u.Long_Double.high_bytes) << 32) + cp_a0->u.Long_Double.low_bytes;
                        fprintf(saida,"\t<%" PRId64 ">\n", bits64);
                        break;
                    case CONSTANT_Float: 

                        // int s = ((bits >> 31) == 0) ? 1 : -1;
                        // int e = ((bits >> 23) & 0xff);
                        // int m = (e == 0) ?
                        //           (bits & 0x7fffff) << 1 :
                        //           (bits & 0x7fffff) | 0x800000;

                        bits32 = (int32_t) constPool->u.Integer_Float.bytes;
                        fprintf(saida, "\t<%" PRId32 ">\n", bits32);
                        
                        switch(bits32)
                        {
                            // If bits is 0x7f800000, the float value will be positive infinity. 
                            case 0x7f800000:
                                fprintf(saida, "\n\tFloat:\t\t+infinity\n");
                                break;
                            // If bits is 0xff800000, the float value will be negative infinity. 
                            case 0xff800000:
                                fprintf(saida, "\n\tFloat:\t\t-infinity\n");
                                break;
                            default:
                                // If bits is in the range 0x7f800001 through 0x7fffffff or in the range 0xff800001 
                                // through 0xffffffff, the float value will be NaN.  
                                if((bits32 >= 0x7f800001 && bits32 <= 0x7fffffff) ||
                                   (bits32 >= 0xff800001 && bits32 <= 0xffffffff )){
                                    fprintf(saida, "\n\tFloat:\t\tNaN\n");
                                }
                                // In all other cases, let s, e, and m be three values that might be computed from bits: 
                                else{
                                    int32_t s = ((bits32 >> 31) == 0) ? 1 : -1;
                                    int32_t e = ((bits32 >> 23) & 0xff);
                                    int32_t m = (e == 0) ?
                                            (bits32 & 0x7fffff) << 1 :
                                            (bits32 & 0x7fffff) | 0x800000;
                                    // Then the float value equals the result of the mathematical expression s · m · 2e-150. 
                                    fprintf(saida, "\n\tFloat:\t\t%f\n", (double) s*m*pow(2, (e-150)));
                                }
                        }
                        break;                          
                    case CONSTANT_Double:
                    // the value represented by the CONSTANT_Double_info structure is determined as follows. The high_bytes and low_bytes items are converted into the long constant bits
                        bits64 = (((int64_t) cp_a0->u.Long_Double.high_bytes) << 32) + cp_a0->u.Long_Double.low_bytes;
                        switch(bits64)
                        {// if bits is 0x7ff0000000000000L, the double value will be positive infinity. 
                            case 0x7ff0000000000000L:
                                fprintf(saida, "\t<+infinity>\n");
                                break;
                        // if bits is 0xfff0000000000000L, the double value will be negative infinity.       
                            case 0xfff0000000000000L:
                                fprintf(saida, "\t<-infinity>\n");
                                break;
                            default:
                            //If bits is in the range 0x7ff0000000000001L through 0x7fffffffffffffffL
                            // or in the range 0xfff0000000000001L through 0xffffffffffffffffL, the double value will be NaN. 
                                if((bits64 >= 0x7ff0000000000001L && bits64 <= 0x7ffffffffffffL) ||
                                   (bits64 >= 0xfff0000000000001L && bits64 <= 0xffffffffffffffffL ))
                                {
                                       fprintf(saida, "\t<NaN>\n");
                                } //In all other cases, let s, e, and m be three values that might be computed from bits: 
                                else
                                {
                                    int32_t s = ((bits64 >> 63) == 0) ? 1 : -1;
                                    int32_t e = ((bits64 >> 52) & 0x7ffL);
                                    int64_t m = (e == 0) ?
                                    (bits64 & 0xfffffffffffffL) << 1 :
                                    (bits64 & 0xfffffffffffffL) |
                                    0x10000000000000L;
                                    fprintf(saida, "\t<%f>\n",
                                            (double) s*m*pow(2, (e-1075)));
                                }//Then the floating-point value equals the double value of the mathematical expression s · m · 2e-1075. 
                        }
                            break;
                    case CONSTANT_String:
                    fprintf(saida,"\t<");
                    printConstantString(cf, cp_a0, saida);
                    fprintf(saida,">\n");
                    break;
                }
                break;
            case CODE:
                fprintf(saida, "\t\tBytecode:\n");
                fprintf(saida,"\t\t\tline\toffset\n");
                uint8_t* nextCode;
                uint32_t lineNumber = 1;
                uint16_t index;
                uint32_t i;
                int32_t default_, n;
                int16_t const_;
                
                for(uint32_t offset = 0; offset < attributes_aux->u.Code.code_length;)
                {
                    fprintf(saida,"\t\t\t%" PRIu32 "\t%" PRIu32"\t", lineNumber++,offset);
                    nextCode = attributes_aux->u.Code.code + offset;
                    // https://en.wikipedia.org/wiki/Java_bytecode_instruction_listings
                    switch(*nextCode){
                        // complemento 0 bytes
                        case aaload:
                        case aastore:
                        case aconst_null:
                        case aload_0:
                        case aload_1:
                        case aload_2:
                        case aload_3:
                        case areturn:
                        case arraylength:
                        case astore_0:
                        case astore_1:
                        case astore_2:
                        case astore_3:
                        case athrow:
                        case baload:
                        case bastore:
                        case breakpoint:
                        case caload:
                        case castore:
                        case d2f:
                        case d2i:
                        case d2l:
                        case dadd:
                        case daload:
                        case dastore:
                        case dcmpg:
                        case dcmpl:
                        case dconst_0:
                        case dconst_1:
                        case ddiv:
                        case dload_0:
                        case dload_1:
                        case dload_2:
                        case dload_3:
                        case dmul:
                        case dneg:
                        case drem:
                        case dreturn:
                        case dstore_0:
                        case dstore_1:
                        case dstore_2:
                        case dstore_3:
                        case dsub:
                        case dup:
                        case dup_x1:
                        case dup_x2:
                        case dup2:
                        case dup2_x1:
                        case dup2_x2:
                        case f2d:
                        case f2i:
                        case f2l:
                        case fadd:
                        case faload:
                        case fastore:
                        case fcmpg:
                        case fcmpl:
                        case fconst_0:
                        case fconst_1:
                        case fconst_2:
                        case fdiv:
                        case fload_0:
                        case fload_1:
                        case fload_2:
                        case fload_3:
                        case fmul:
                        case fneg:
                        case frem:
                        case freturn:
                        case fstore_0:
                        case fstore_1:
                        case fstore_2:
                        case fstore_3:
                        case fsub:
                        case i2b:
                        case i2c:
                        case i2d:
                        case i2f:
                        case i2l:
                        case i2s:
                        case iadd:
                        case iaload:
                        case iand:
                        case iastore:
                        case iconst_m1:
                        case iconst_0:
                        case iconst_1:
                        case iconst_2:
                        case iconst_3:
                        case iconst_4:
                        case iconst_5:
                        case idiv:
                        case iload_0:
                        case iload_1:
                        case iload_2:
                        case iload_3:
                        case impdep1:
                        case impdep2:
                        case imul:
                        case ineg:
                        case ior:
                        case irem:
                        case ireturn:
                        case ishl:
                        case ishr:
                        case istore_0:
                        case istore_1:
                        case istore_2:
                        case istore_3:
                        case isub:
                        case iushr:
                        case ixor:
                        case l2d:
                        case l2f:
                        case l2i:
                        case ladd:
                        case laload:
                        case land:
                        case lastore:
                        case lcmp:
                        case lconst_0:
                        case lconst_1:
                        case ldiv_:
                        case lload_0:
                        case lload_1:
                        case lload_2:
                        case lload_3:
                        case lmul:
                        case lneg:
                        case lor:
                        case lrem:
                        case lreturn:
                        case lshl:
                        case lshr:
                        case lstore_0:
                        case lstore_1:
                        case lstore_2:
                        case lstore_3:
                        case lsub:
                        case lushr:
                        case lxor:
                        case monitorenter:
                        case monitorexit:
                        case nop:
                        case pop:
                        case pop2:
                        case return_:
                        case saload:
                        case sastore:
                        case swap:
                            fprintf(saida, "%s\n", op_codesJVM[*nextCode]);
                            // incrementa o iterador
                            offset++;
                            break;
                            // complemento 1 byte
                            //  local variable index (uint8_t)
                        case aload:
                        case astore:
                        case dload:
                        case dstore:
                        case fload:
                        case fstore:
                        case iload:
                        case istore:
                        case lload:
                        case lstore:
                        case ret:
                            fprintf(saida, "%s", op_codesJVM[*nextCode]);
                            fprintf(saida, "\t%" PRIu8 "\n", *(nextCode+1));
                            // incrementa o iterador 2 vezes
                            offset +=2;
                            break;
                            
                            //  signed integer (s1)
                        case bipush:
                            fprintf(saida, "%s", op_codesJVM[*nextCode]);
                            fprintf(saida, "\t%" PRId8 "\n", *(nextCode+1));
                            offset +=2;
                            break;
                            
                            //  array type (uint8_t)
                        case newarray:
                            fprintf(saida, "%s\t", op_codesJVM[*nextCode]);
                            fprintf(saida,"%" PRId8, *(nextCode+1));
                            switch(*(nextCode+1)){
                                // printa o tipo de array
                                case T_BOOLEAN:
                                    fprintf(saida," (boolean)\n");
                                    break;
                                case T_CHAR:
                                    fprintf(saida," (char)\n");
                                    break;
                                case T_FLOAT:
                                    fprintf(saida," (float)\n");
                                    break;
                                case T_DOUBLE:
                                    fprintf(saida," (double)\n");
                                    break;
                                case T_BYTE:
                                    fprintf(saida," (byte)\n");
                                    break;
                                case T_SHORT:
                                    fprintf(saida," (short)\n");
                                    break;
                                case T_INT:
                                    fprintf(saida," (int)\n");
                                    break;
                                case T_LONG:
                                    fprintf(saida," (long\n");
                                    break;
                                default:
                                    printf("[ERRO] newarray aplicado tipo invalido.\n");
                                    exit(EXIT_FAILURE);
                                    break;
                            }
                            offset +=2;
                            break;
                        
                        //  CONSTANTE pool index (integer, float, string) (uint8_t)
                    case ldc:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        fprintf(saida, "\t#%" PRIu8, *(nextCode+1));
                        cp_a0 = cf->constant_pool + *(nextCode+1) - 1;
                        switch(cp_a0->tag){
                            case CONSTANT_Integer:
                                fprintf(saida, "\t<%" PRId32 ">\n", cp_a0->u.Integer_Float.bytes);
                                break;
                            case CONSTANT_Float:
                                ;
                                float   float_number;
                                memcpy(&float_number,&cp_a0->u.Integer_Float.bytes, sizeof(uint32_t));
                                fprintf(saida, "\t<%f>\n", float_number);
                                break;
                            case CONSTANT_String:
                                fprintf(saida,"\t<");
                                printConstantString(cf, cp_a0, saida);
                                fprintf(saida,">\n");
                                break;
                            default:
                                puts("[ERRO] instrucao ldc com argumento invalido\n");
                                exit(EXIT_FAILURE);
                        }
                        offset +=2;
                        break;
                        
                        // 2 bytes
                        //  constant_pool index (class) (uint16_t)
                    case anewarray:
                    case checkcast:
                    case instanceof:
                    case new:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        index = (uint16_t) (*(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t#%" PRIu16, index);
                        cp_a0 = cf->constant_pool + index - 1;
                        if(cp_a0->tag != CONSTANT_Class){
                            printf("\n[ERRO] %s nao referencia um CONSTANT_Class valido.\n",
                                   op_codesJVM[*nextCode]);
                            exit(EXIT_FAILURE);
                        }
                        else{
                            uint16_t    name_index = cp_a0->u.Class.name_index;
                            cp_a0 = cf->constant_pool + name_index - 1;
                            
                            fprintf(saida, "\t<");
                            printConstUtf8(cp_a0, saida);
                            fprintf(saida, ">\n");
                        }
                        offset +=3;
                        break;
                        
                        //  branch offset (s2)
                    case goto_:
                    case if_acmpeq:
                    case if_acmpne:
                    case if_icmpeq:
                    case if_icmpge:
                    case if_icmpgt:
                    case if_icmple:
                    case if_icmplt:
                    case if_icmpne:
                    case ifeq:
                    case ifge:
                    case ifgt:
                    case ifle:
                    case iflt:
                    case ifne:
                    case ifnonnull:
                    case ifnull:
                    case jsr:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        branch = ( (int8_t) *(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t%" PRId32 "\t(%+" PRId32 ")\n", (int32_t) offset + branch, branch);
                        offset +=3;
                        break;
                        
                        //  CONSTANTE pool index (integer, float, string) (uint16_t)
                    case ldc_w:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        index = (uint16_t) (*(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t#%" PRIu16 "\n", index);
                        cp_a0 = cf->constant_pool + *(nextCode+1) - 1;
                        switch(cp_a0->tag){
                            case CONSTANT_Integer:
                                fprintf(saida, "\t<%" PRId32 ">\n", cp_a0->u.Integer_Float.bytes);
                                break;
                            case CONSTANT_Float:
                                ;
                                float   float_number;
                                memcpy(&float_number,&cp_a0->u.Integer_Float.bytes, sizeof(uint32_t));
                                fprintf(saida, "\t<%f>\n", float_number);
                                break;
                            case CONSTANT_String:
                                fprintf(saida,"\t<");
                                printConstantString(cf, cp_a0, saida);
                                fprintf(saida,">\n");
                                break;
                            default:
                                puts("ERRO: instrucao ldc_w com argumento invalido\n");
                                exit(EXIT_FAILURE);
                        }
                        offset +=3;
                        break;
                        
                        //  CONSTANTE pool index (fieldref; methodref) (uint16_t)
                    case getfield:
                    case getstatic:
                    case invokespecial:
                    case invokestatic:
                    case invokevirtual:
                    case putfield:
                    case putstatic:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        index = (uint16_t) (*(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t#%" PRIu16, index);
                        cp_a0 = cf->constant_pool + index - 1;
                        if(cp_a0->tag != CONSTANT_Methodref && cp_a0->tag != CONSTANT_Fieldref){
                            printf("\nERRO: %s nao referencia um CONSTANTE_ref valido.\n", op_codesJVM[*nextCode]);
                            exit(EXIT_FAILURE);
                        }
                        else{
                            uint16_t    name_index = cp_a0->u.Ref.name_index;
                            uint16_t    name_and_type_index = cp_a0->u.Ref.name_and_type_index;
                            
                            cp_a0 = cf->constant_pool + name_index - 1;
                            if(cp_a0->tag != CONSTANT_Class){
                                printf("\nERRO: %s nao referencia um CONSTANTE_ref valido.\n", op_codesJVM[*nextCode]);
                                exit(EXIT_FAILURE);
                            }
                            else{
                                uint16_t name_index = cp_a0->u.Class.name_index;
                                cp_a0 = cf->constant_pool + name_index - 1;
                                
                                fprintf(saida, "\t<");
                                printConstUtf8(cp_a0, saida);
                                
                                cp_a0 = cf->constant_pool + name_and_type_index - 1;
                                if(cp_a0->tag != CONSTANT_NameAndType){
                                    printf("\nERRO: %s nao referencia um CONSTANTE_ref valido.\n",
                                           op_codesJVM[*nextCode]);
                                    exit(EXIT_FAILURE);
                                }
                                else{
                                    name_index = cp_a0->u.NameAndType.name_index;
                                    cp_a0 = cf->constant_pool + name_index - 1;
                                    
                                    fprintf(saida, ".");
                                    printConstUtf8(cp_a0, saida);
                                    fprintf(saida, ">\n");
                                }
                            }
                            
                        }
                        offset +=3;
                        break;
                        
                        //  CONSTANTE pool index (long, double) (uint16_t)
                    case ldc2_w:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        index = (uint16_t) (*(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t#%" PRIu16, index);
                        cp_a0 = cf->constant_pool + index - 1;
                        switch(cp_a0->tag){
                            case CONSTANT_Long:
                                bits64 = (((int64_t) cp_a0->u.Long_Double.high_bytes) << 32)
                                + cp_a0->u.Long_Double.low_bytes;
                                fprintf(saida,"\t<%" PRId64 ">\n", bits64);
                                break;
                            case CONSTANT_Double:
                                bits64 = (((int64_t) cp_a0->u.Long_Double.high_bytes) << 32)
                                + cp_a0->u.Long_Double.low_bytes;
                                switch(bits64){
                                    case 0x7ff0000000000000L:
                                        fprintf(saida, "\t<+8>\n");
                                        break;
                                    case 0xfff0000000000000L:
                                        fprintf(saida, "\t<-8>\n");
                                        break;
                                    default:
                                        if((bits64 >= 0x7ff0000000000001L &&
                                            bits64 <= 0x7ffffffffffffL) ||
                                           (bits64 >= 0xfff0000000000001L &&
                                            bits64 <= 0xffffffffffffffffL )){
                                               fprintf(saida, "\t<NaN>\n");
                                           }
                                        else{
                                            int32_t s = ((bits64 >> 63) == 0) ? 1 : -1;
                                            int32_t e = ((bits64 >> 52) & 0x7ffL);
                                            int64_t m = (e == 0) ?
                                            (bits64 & 0xfffffffffffffL) << 1 :
                                            (bits64 & 0xfffffffffffffL) |
                                            0x10000000000000L;
                                            fprintf(saida, "\t<%f>\n",
                                                    (double) s*m*pow(2, (e-1075)));
                                        }
                                }
                                break;
                            default:
                                puts("[ERRO] instrucao ldc2_w com argumento invalido\n");
                                exit(EXIT_FAILURE);
                        }
                        offset +=3;
                        break;
                        
                        //  signed short (s2)
                    case sipush:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        fprintf(saida, "\t%" PRId16 "\n", *(nextCode+1));
                        offset +=3;
                        break;
                        
                        //  unsigned byte (uint8_t); signed byte (s1)
                    case iinc: // 1 byte 1 byte
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        fprintf(saida, "\t%" PRIu8 , *(nextCode+1));
                        fprintf(saida, " by %" PRId8 "\n", *(nextCode+2));
                        offset +=3;
                        break;
                        // 3 bytes
                        // constant_pool index (class) (uint16_t) ; unsigned byte (uint8_t)
                    case multianewarray:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        index = (uint16_t) (*(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t#%" PRIu16, index);
                        cp_a0 = cf->constant_pool + index - 1;
                        if(cp_a0->tag != CONSTANT_Class){
                            printf("\n[ERRO] %s nao referencia um CONSTANT_Class valido.\n",
                                   op_codesJVM[*nextCode]);
                            exit(EXIT_FAILURE);
                        }
                        else{
                            uint16_t    name_index = cp_a0->u.Class.name_index;
                            cp_a0 = cf->constant_pool + name_index - 1;
                            
                            fprintf(saida, "\t<");
                            printConstUtf8(cp_a0, saida);
                            fprintf(saida, ">");
                        }
                        fprintf(saida, "\tdim %" PRIu8 "\n", *(nextCode+3));
                        offset +=4;
                        break;
                        
                        // 4 bytes
                        // branch offset (uint32_t)
                    case goto_w:
                    case jsr_w:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        branch = ((uint32_t) *(nextCode+1) << 24) + ((uint32_t) *(nextCode+2) << 16) +
                        ((uint32_t) *(nextCode+3) << 8) + *(nextCode+4);
                        fprintf(saida, "\t%" PRIu32 "\t(+%" PRIu32 ")\n", offset + branch, branch);
                        offset +=5;
                        break;
                        
                        // constant_pool index (uint16_t); unsigned byte (uint8_t); unsigned byte (uint8_t)
                    case invokeinterface:
                    case invokedynamic:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        index = (uint16_t) (*(nextCode+1) << 8) + *(nextCode+2);
                        fprintf(saida, "\t#%" PRIu16, index);
                        cp_a0 = cf->constant_pool + index - 1;
                        if(cp_a0->tag != CONSTANT_InterfaceMethodref && cp_a0->tag != CONSTANT_Methodref){
                            printf("\n[ERRO] %s nao referencia um CONSTANT_ref valido.\n",
                                   op_codesJVM[*nextCode]);
                            exit(EXIT_FAILURE);
                        }
                        else{
                            uint16_t    name_index = cp_a0->u.Ref.name_index;
                            uint16_t    name_and_type_index = cp_a0->u.Ref.name_and_type_index;
                            
                            cp_a0 = cf->constant_pool + name_index - 1;
                            if(cp_a0->tag != CONSTANT_Class){
                                printf("\n[ERRO] %s nao referencia um CONSTANT_ref valido.\n",
                                       op_codesJVM[*nextCode]);
                                exit(EXIT_FAILURE);
                            }
                            else{
                                uint16_t    name_index = cp_a0->u.Class.name_index;
                                cp_a0 = cf->constant_pool + name_index - 1;
                                
                                fprintf(saida, "\t<");
                                printConstUtf8(cp_a0, saida);
                                
                                cp_a0 = cf->constant_pool + name_and_type_index - 1;
                                if(cp_a0->tag != CONSTANT_NameAndType){
                                    printf("\n[ERRO] %s nao referencia um CONSTANT_ref valido.\n",op_codesJVM[*nextCode]);
                                    exit(EXIT_FAILURE);
                                }
                                else{
                                    name_index = cp_a0->u.NameAndType.name_index;
                                    cp_a0 = cf->constant_pool + name_index - 1;
                                    
                                    fprintf(saida, ".");
                                    printConstUtf8(cp_a0, saida);
                                    fprintf(saida, ">");
                                }
                                fprintf(saida,"\t<%" PRIu8  ">\n", *(nextCode+3));
                            }
                        }
                        offset +=5;
                        break;
                        
                        // 4 ou + bytes
                        //  <0-3 byte pad>; default (s4); n (s4); key1, offobtain_label1 ... keyn, offobtain_labeln (s4)
                    case lookupswitch:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        i = (offset+1) % 4;
                        default_ = (*((1+i) + nextCode) << 24) | (*((2+i) + nextCode) << 16) |
                        (*((3+i) + nextCode) << 8) | *((4+i) + nextCode);
                        
                        n = (*((5+i) + nextCode) << 24) | (*((6+i) + nextCode) << 16) |
                        (*((7+i) + nextCode) << 8) | *((8+i) + nextCode);
                        i += 8;
                        fprintf(saida,"\t%" PRId32 "\n", n);
                        
                        int32_t key, offobtain_label;
                        for(uint32_t j = 0; j < n; j++){
                            fprintf(saida,"\t\t\t\t\t%" PRIu32, lineNumber++);
                            key = (*((1+i) + nextCode) << 24) | (*((2+i) + nextCode) << 16) |
                            (*((3+i) + nextCode) << 8) | *((4+i) + nextCode);
                            i += 4;
                            fprintf(saida, "\t\t\t%" PRId32, key);
                            offobtain_label = (*((1+i) + nextCode) << 24) | (*((2+i) + nextCode) << 16) |
                            (*((3+i) + nextCode) << 8) | *((4+i) + nextCode);
                            i += 4;
                            fprintf(saida, ":\t%" PRId32 " (%+" PRId32 ")\n",
                                    offset + offobtain_label, offobtain_label);
                        }
                        fprintf(saida, "\t\t\t\t\t%" PRIu32, lineNumber++);
                        fprintf(saida, "\t\t\tdefault:\t%" PRId32 " (+%" PRId32 ")\n",
                                offset + default_, default_);
                        offset +=i;
                        offset++;
                        break;
                        
                        //  <0-3 byte pad>; default (s4); low (s4); high (s4); label1 ... labeln (s4)
                    case tableswitch:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        i = (offset+1) % 4;
                        default_ = (*((1+i) + nextCode) << 24) | (*((2+i) + nextCode) << 16) |
                        (*((3+i) + nextCode) << 8) | *((4+i) + nextCode);
                        
                        int32_t low = (*((5+i) + nextCode) << 24) | (*((6+i) + nextCode) << 16) |
                        (*((7+i) + nextCode) << 8) | *((8+i) + nextCode);
                        
                        int32_t high = (*((9+i) + nextCode) << 24) | (*((10+i) + nextCode) << 16) |
                        (*((11+i) + nextCode) << 8) | *((12+i) + nextCode);
                        
                        i += 12;
                        fprintf(saida, "\t%" PRId32 " to %" PRId32 "\n", low, high);
                        
                        n = high - low + 1;
                        
                        int32_t label;
                        for(uint32_t j = 0; j < n; j++){
                            fprintf(saida,"\t\t\t\t\t%" PRIu32, lineNumber++);
                            label = (*((1+i) + nextCode) << 24) | (*((2+i) + nextCode) << 16) |
                            (*((3+i) + nextCode) << 8) | *((4+i) + nextCode);
                            i += 4;
                            fprintf(saida, "\t\t\t%" PRId32 ":\t%" PRId32 " (%+" PRId32 ")\n",
                                    j, offset + label, label);
                        }
                        fprintf(saida, "\t\t\t\t\t%" PRIu32, lineNumber++);
                        fprintf(saida, "\t\t\tdefault:\t%" PRId32 " (%+" PRId32 ")\n",
                                offset + default_, default_);
                        offset +=i;
                        offset++;
                        break;
                        // 3 ou 5 bytes
                    case wide:
                        fprintf(saida, "%s", op_codesJVM[*nextCode]);
                        fprintf(saida,"\t\t\t\t\t%" PRIu32, lineNumber++);
                        fprintf(saida, "%s", op_codesJVM[*(nextCode+1)]);
                        index = (uint16_t) (*(nextCode+2) << 8) + *(nextCode+3);
                        fprintf(saida, "\t%" PRIu16, index);
                        switch(*(nextCode+1)){
                            case aload:
                            case dload:
                            case iload:
                            case fload:
                            case lload:
                            case astore:
                            case dstore:
                            case istore:
                            case fstore:
                            case lstore:
                            case ret:
                                offset +=4;
                                break;
                            case iinc:
                                const_ = (uint16_t) (*(nextCode+4) << 8) + *(nextCode+5);
                                fprintf(saida,"\t%" PRId16, const_);
                                offset +=6;
                                break;
                            default:
                                printf("[ERRO] wide aplicado a uma instrucao invalida\n");
                                exit(EXIT_FAILURE);
                        }
                        fprintf(saida, "\n");
                        break;
                }
            }
            fprintf(saida, "\n\t\t\tException Table Length:\t\t%" PRId16 "\n", attributes_aux->u.Code.exception_table_length);
            if(attributes_aux->u.Code.exception_table_length){
                fprintf(saida, "\n\t\t\tException table:\n");
                fprintf(saida, "\t\t\tNr.\tstart_pc\tend_pc\thandler_pc\tcatch_type\tverbose\n");
                for(uint16_t i = 0; i < attributes_aux->u.Code.exception_table_length; i++){
                    fprintf(saida,"\t\t\t%" PRId16, i);
                    fprintf(saida,"\t%" PRId16,
                            (attributes_aux->u.Code.exception_table + i)->start_pc);
                    fprintf(saida,"\t%" PRId16,
                            (attributes_aux->u.Code.exception_table + i)->end_pc);
                    fprintf(saida,"\t%" PRId16,
                            (attributes_aux->u.Code.exception_table + i)->handler_pc);
                    fprintf(saida,"\tcp_info #%" PRId16,
                            (attributes_aux->u.Code.exception_table + i)->catch_type);
                    constant_pool_info  * cp_a0 = cf->constant_pool + (attributes_aux->u.Code.exception_table + i)->catch_type - 1;
                    printConstantClass(cf, cp_a0, saida);
                    fprintf(saida,"\n");
                    
                }
            }
            fprintf(saida, "\n\t\tMiscellaneous:\n");
            fprintf(saida, "\t\t\t\tMax Stack:\t\t%" PRId16 "\n", attributes_aux->u.Code.max_stack);
            fprintf(saida, "\t\t\t\tMax locals:\t%" PRId16 "\n", attributes_aux->u.Code.max_locals);
            fprintf(saida, "\t\t\t\tCode length:\t\t\t%" PRId32 "\n", attributes_aux->u.Code.code_length);
            fprintf(saida,"\n\t\tCode attributes count:\t%" PRId16 "\n", attributes_aux->u.Code.attributes_count);
            showAttributes(NULL, NULL, attributes_aux, cf, saida);
            break;
            case DEPRECATED:
                /* */
                break;
            case EXCEPTIONS:
                fprintf(saida, "\t\t\tNumber of exceptions: %" PRId16, attributes_aux->u.Exceptions.number_of_exceptions);
                fprintf(saida,"\t\t\tTable of Exceptions index:\n");
                for(uint16_t i = 0; i < attributes_aux->u.Exceptions.number_of_exceptions; i++){
                    fprintf(saida,"\t\t\t constant_pool_info #%" PRId16 "\n", attributes_aux->u.Exceptions.exception_index_table[i]);
                }
                break;
            case INNER_CLASSES:
                fprintf(saida, "\t\tNumber of classes: %" PRId16 "\n", attributes_aux->u.InnerClasses.number_of_classes);
                fprintf(saida, "\t\t\tClasses:\n");
                classes_type    * classes_aux = attributes_aux->u.InnerClasses.classes;
                for(uint16_t i = 0; i < attributes_aux->u.InnerClasses.number_of_classes; i++){
                    fprintf(saida, "\t\t\t\tClass %" PRId16 "\n", i);
                    fprintf(saida, "\t\t\t\t\tinner_class_info_index:\t\tcp_info #%" PRId16 "\t",
                            (classes_aux)->inner_class_info_index);
                    printConstantClass(cf, cf->constant_pool +
                                       (classes_aux)->inner_class_info_index - 1, saida);
                    fprintf(saida, "\n\t\t\t\t\tsuper_class_info_index:\t\tcp_info #%" PRId16 "\t",
                            (classes_aux)->outer_class_info_index);
                    printConstantClass(cf, cf->constant_pool +
                                       (classes_aux)->outer_class_info_index - 1, saida);
                    fprintf(saida, "\n\t\t\t\t\tinner_name_index:\t\tcp_info #%" PRId16 "\t",
                            (classes_aux)->inner_name_index);
                    printConstUtf8(cf->constant_pool +
                                     (classes_aux)->inner_name_index - 1, saida);
                    fprintf(saida, "\n\t\t\t\t\tinner_class_access_flags:\t0x%.4" PRIx16 "\t\t",
                            (classes_aux)->inner_class_access_flags);
                    uint16_t    access_flags = (classes_aux)->inner_class_access_flags;
                    fprintf(saida, "[");
                    if(access_flags >= ACC_ABSTRACT){
                        fprintf(saida, "abstract ");
                        access_flags -= ACC_ABSTRACT;
                    }
                    if(access_flags >= ACC_INTERFACE){
                        fprintf(saida, "interface ");
                        access_flags -= ACC_INTERFACE;
                    }
                    if(access_flags >= ACC_FINAL){
                        fprintf(saida, "final ");
                        access_flags -= ACC_FINAL;
                    }
                    if(access_flags >= ACC_STATIC){
                        fprintf(saida, "static ");
                        access_flags -= ACC_STATIC;
                    }
                    if(access_flags >= ACC_PROTECTED){
                        fprintf(saida, "protected ");
                        access_flags -= ACC_PROTECTED;
                    }
                    if(access_flags >= ACC_PRIVATE){
                        fprintf(saida, "private ");
                        access_flags -= ACC_PRIVATE;
                    }
                    if(access_flags >= ACC_PUBLIC){
                        fprintf(saida, "public");
                        access_flags -= ACC_PUBLIC;
                    }
                    fprintf(saida, "]\n");
                    classes_aux++;
                }
                break;
            case LINE_NUMBER_TABLE:
                fprintf(saida, "\t\t\t\tLine Number Table length:\t%" PRId16 "\n", attributes_aux->u.LineNumberTable.line_number_table_length);
                fprintf(saida, "\t\t\t\tLine Number Table:\n");
                fprintf(saida, "\t\t\t\tNr.\tstart_pc\tline_number\n");
                for(uint16_t i = 0; i < attributes_aux->u.LineNumberTable.line_number_table_length; i++){
                    fprintf(saida,"\t\t\t\t%" PRId16, i);
                    fprintf(saida,"\t%" PRId16, ((attributes_aux->u.LineNumberTable.line_number_table)+i)->start_pc);
                    fprintf(saida,"\t%" PRId16 "\n",
                            ((attributes_aux->u.LineNumberTable.line_number_table)+i)->line_number);            
                }
                break;
            case LOCAL_VARIABLE_TABLE:
                fprintf(saida, "\t\t\tLocal variable table length:\t%" PRId16 "\n",
                        attributes_aux->u.LocalVariableTable.local_variable_table_length);
                fprintf(saida, "\t\t\tLocal variables table:\n");
                fprintf(saida, "\t\t\tNr.\tstart_pc\tlength\tindex\tname_index\tdescriptor_index\n");
                for(uint16_t i = 0; i < attributes_aux->u.LocalVariableTable.local_variable_table_length; i++){
                    fprintf(saida,"\t\t\t%" PRId16, i);
                    fprintf(saida,"\t%" PRId16,
                            ((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->start_pc);     
                    fprintf(saida,"\t%" PRId16,
                            ((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->length);
                    fprintf(saida,"\t%" PRId16,
                            ((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->index);
                    fprintf(saida,"\tcp_info #%" PRId16,
                            ((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->name_index);
                    fprintf(saida,"\tcp_info #%" PRId16 "\n",
                            ((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->descriptor_index);
                    cp_a0 = cf->constant_pool + (((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->name_index) - 1;
                    fprintf(saida,"\t\t\t\t\t\t\t\t");
                    printConstUtf8(cp_a0, saida);
                    fprintf(saida,"\t");
                    cp_a0 = cf->constant_pool + (((attributes_aux->u.LocalVariableTable.local_variable_table)+i)->descriptor_index) - 1;
                    printConstUtf8(cp_a0, saida);   
                    fprintf(saida, "\n");
                    
                }
                break;
            case SOURCE_FILE:
                fprintf(saida, "\t\tSource file index:\tcp_info #%" PRId16, attributes_aux->u.SourceFile.sourcefile_index);
                uint16_t    sourcefile_index = attributes_aux->u.SourceFile.sourcefile_index;
                cp_a0 = cf->constant_pool + sourcefile_index - 1;
                if(cp_a0->tag != CONSTANT_Utf8){
                    puts("\n[ERRO] SourceFile Attribute nao referencia um nome valido.\n");
                    exit(EXIT_FAILURE);
                }
                else{
                    fprintf(saida, "\t");
                    printConstUtf8(cp_a0, saida);
                    fprintf(saida, "\n");   
                }
                break;
            case UNKNOWN:
                /*              // IGNORANDO ATRIBUTOS DESCONHECIDOS*/
                break;          
        }
        attributes_aux++;
    }
}


void printConstUtf8(constant_pool_info * constPool, FILE * saida)
{
    char * string = (char *) constPool->u.Utf8.bytes;
    string[constPool->u.Utf8.length] = '\0';
    fprintf(saida, "%s", string );
}

// imprime no arquivo de saida os valores apontados pelo CONSTANT_Class
void printConstantClass(ClassFile * cf, constant_pool_info * constPool, FILE * saida)
{
    constant_pool_info * cp_a0;
    cp_a0 = cf->constant_pool + constPool->u.Class.name_index - 1;
    // se nao eh CONTANT_Utf8
    if(cp_a0->tag != CONSTANT_Utf8)
    {
        puts("\n[ERRO] valor do name_index de CONSTANT_Class_info nao eh indice de cp_info cuja tag eh CONSTANT_Utf8\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printConstUtf8(cp_a0, saida);
    }
}

// imprime no arquivo de saida os valores apontados pelos CONSTANT_Fieldref, CONSTANT_Methodref, CONSTANT_InterfaceMethodref
void printConstantRef(ClassFile * cf, constant_pool_info * constPool,char tipo_indicador, FILE * saida)
{
    constant_pool_info * cp_a0;
    constant_pool_info * cp_a2;
    switch(tipo_indicador)
    {
        // se campo eh Class name
        case 'c':
            cp_a0 = cf->constant_pool + constPool->u.Ref.name_index - 1;
            
            // se nao eh CONSTANT_Class
            if(cp_a0->tag != CONSTANT_Class)
            {
                puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_Class)\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                cp_a0 = cf->constant_pool + cp_a0->u.Class.name_index - 1;
                if(cp_a0->tag != CONSTANT_Utf8)
                {
                    puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_Utf8)\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printConstUtf8(cp_a0, saida);
                }
            }
            break;
            
        // se campo eh Name and type
        case 'n':
            cp_a0 = cf->constant_pool + constPool->u.Ref.name_and_type_index - 1;
            
            // se nao eh CONSTANT_NameAndType
            if(cp_a0->tag != CONSTANT_NameAndType)
            {
                puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_NameAndType)\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                cp_a2 = cp_a0;
                cp_a0 = cf->constant_pool + cp_a0->u.NameAndType.name_index - 1;
                if(cp_a0->tag != CONSTANT_Utf8)
                {
                    puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_Utf8)\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printConstUtf8(cp_a0, saida);
                }
                
                cp_a2 = cf->constant_pool + cp_a2->u.NameAndType.descriptor_index - 1;
                if(cp_a2->tag != CONSTANT_Utf8)
                {
                    puts("[ERRO] valor da tag invalido (Diferente de CONSTANT_Utf8).\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printConstUtf8(cp_a2, saida);
                }
            }
            break;
    }
    
}

// imprime no arquivo de saida os valores apontados pelo CONSTANT_String
void printConstantString(ClassFile * cf, constant_pool_info * constPool, FILE * saida)
{
    constant_pool_info * cp_a0;
    cp_a0 = cf->constant_pool + constPool->u.String.string_index - 1;
    // se nao for CONSTANT_Utf8
    if(cp_a0->tag != CONSTANT_Utf8)
    {
        puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_Utf8)\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printConstUtf8(cp_a0, saida);
    }
}

// imprime no arquivo de saida os valores apontados pelo CONSTANT_NameType
void printConstantNameAndType(ClassFile * cf, constant_pool_info * constPool, char descritor_nome, FILE * saida)
{
    constant_pool_info * cp_a0 = constPool;
    
    // Name
    if (descritor_nome == 'n') 
    {
        cp_a0 = cf->constant_pool + cp_a0->u.NameAndType.name_index - 1;
        // se nao eh CONSTANT_Utf8   
        if(cp_a0->tag != CONSTANT_Utf8)
        {
            puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_Utf8)\n");
            exit(EXIT_FAILURE);
        }
        // se eh CONSTANT_Utf8 
        else
        {
            printConstUtf8(cp_a0,saida);
        }
    } 
    
    // Descriptor
    else if (descritor_nome == 'd') {
        cp_a0 = cf->constant_pool + cp_a0->u.NameAndType.descriptor_index- 1;
            if(cp_a0->tag != CONSTANT_Utf8)
            {
                puts("\n[ERRO] valor da tag invalido (Diferente de CONSTANT_Utf8).\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                for(uint16_t i = 0; i < cp_a0->u.Utf8.length; i++)
                {
                    fprintf(saida, "%c" , cp_a0->u.Utf8.bytes[i]);
                }
            }
    } 
}

// libera a memoria que fora alocada para class file
void unloadClassFile(ClassFile * cf)
{
    if(!cf){
        return;
    }
    // libera as interfaces
    free(cf->interfaces);
    // libera os fields - precisa de fc pq tem que liberar os attributes primeiro
    unloadFields(cf);
    // libera os fields - precisa de fc pq tem que liberar os attributes primeiro
    unloadMethods(cf);

    // libera os attributes - precisa de fc pq tem que liberar os attributes primeiro
    for(attribute_info * attr = cf->attributes; attr < (cf->attributes + cf->attributes_count); attr++){
        unloadAttribute(attr, cf);
    }
    if(cf->attributes_count){
        free(cf->attributes);
    }
    // libera a contant pool - precisa de fc pq precisa liberar as CONSTANT_Utf8
    unloadConstantPool(cf); 
    // libera o classfile  
    free(cf);
}


// Liberar a memória alocada para os fields
// Percorre os fields, liberando os attributes e em seguida desaloca o field
void unloadFields(ClassFile * cf)
{
    // para cada field ate field +field_count
    for(field_info * fd_in = cf->fields; fd_in < (cf->fields + cf->fields_count); fd_in++){
        for(attribute_info * attr = fd_in->attributes; attr < (fd_in->attributes + fd_in->attributes_count); attr++)
        {
            unloadAttribute(attr, cf);
        }
        if(fd_in->attributes_count){
            free(fd_in->attributes);
        }
    }
    if(cf->fields_count){
        free(cf->fields);
    }
}

// Liberar memória alocada para os methods
// Percorre os methods, chama a função que libera os attibutes e em seguida desaloca o method
void unloadMethods(ClassFile * cf)
{
    for(method_info * mt_in = cf->methods; mt_in < (cf->methods + cf->methods_count); mt_in++)
    { 
        for(attribute_info * attr = mt_in->attributes; attr < (mt_in->attributes + mt_in->attributes_count); attr++)
        {
            unloadAttribute(attr, cf);
        }
       
        if(mt_in->attributes_count)
        {
            free(mt_in->attributes);
        }
    }
    if(cf->methods_count){
        free(cf->methods);
    }
}

// Liberar memória alocada para os attributes
// Verifica o tipo e em seguida libera
void unloadAttribute(attribute_info * attr, ClassFile * cf){
    switch(getAttributeType(attr, cf)){
        case CONSTANT_VALUE:
            break;
        case CODE:
            if(attr->u.Code.code_length){
                free(attr->u.Code.code);                
            }
            
            if(attr->u.Code.exception_table_length){
                free(attr->u.Code.exception_table);
            }
            
            for(attribute_info * attr_ = attr->u.Code.attributes;
                attr_ < (attr->u.Code.attributes + attr->u.Code.attributes_count); attr_++){
                unloadAttribute(attr_, cf);
            }
            if(attr->u.Code.attributes_count){
                free(attr->u.Code.attributes);
            }
            
            break;
        case DEPRECATED:
            break;
        case EXCEPTIONS:
            free(attr->u.Exceptions.exception_index_table);                 
            break;
        case INNER_CLASSES:
            if(attr->u.InnerClasses.number_of_classes){
                free(attr->u.InnerClasses.classes);
            }
            break;
        case LINE_NUMBER_TABLE:
            if(attr->u.LineNumberTable.line_number_table_length){
                free(attr->u.LineNumberTable.line_number_table);
            }
            break;
        case LOCAL_VARIABLE_TABLE:
            if(attr->u.LocalVariableTable.local_variable_table_length){
                free(attr->u.LocalVariableTable.local_variable_table);
            }
            break;
        case SYNTHETIC:
            break;
        case SOURCE_FILE:
            break;
        case UNKNOWN:
            break;                          
    }
}

// Liberar memória alodcada para contant_pool.
// Verifica se o registro é CONSTANT_UTF8 e desaloca
// Após tudo desalocado, decaloca a contant_pool
void unloadConstantPool(ClassFile * cf){

    for(constant_pool_info * constPool = cf->constant_pool; constPool < (cf->constant_pool + cf->constant_pool_count - 1); constPool++)
    {
        if (constPool->tag == CONSTANT_Utf8)
        {
            free(constPool->u.Utf8.bytes);
        }
    }
    free(cf->constant_pool);
}

// array de mnemônicos opcodes.h
char *op_codesJVM[] = 
{
    "nop", "aconst_null", "iconst_m1", "iconst_0", "iconst_1", 
    "iconst_2", "iconst_3", "iconst_4", "iconst_5", "lconst_0",
    "lconst_1", "fconst_0", "fconst_1", "fconst_2", "dconst_0", 
    "dconst_1", "bipush", "sipush", "ldc", "ldc_w", "ldc2_w", 
    "iload", "lload", "fload","dload", "aload", "iload_0", "iload_1",
    "iload_2", "iload_3", "lload_0", "lload_1", "lload_2", "lload_3", 
    "fload_0", "fload_1", "fload_2", "fload_3", "dload_0", "dload_1",
    "dload_2", "dload_3", "aload_0", "aload_1", "aload_2", "aload_3",
    "iaload", "laload", "faload", "daload", "aaload", "baload", "caload",
    "saload", "istore", "lstore", "fstore", "dstore", "astore", "istore_0",
    "istore_1", "istore_2", "istore_3", "lstore_0", "lstore_1", "lstore_2",
    "lstore_3", "fstore_0", "fstore_1", "fstore_2", "fstore_3", "dstore_0",
    "dstore_1", "dstore_2", "dstore_3", "astore_0", "astore_1", "astore_2",
    "astore_3", "iastore", "lastore", "fastore", "dastore", "aastore", "bastore",
    "castore", "sastore", "pop", "pop2", "dup", "dup_x1", "dup_x2", "dup2",
    "dup2_x1", "dup2_x2", "swap", "iadd", "ladd", "fadd", "dadd", "isub",
    "lsub", "fsub", "dsub", "imul", "lmul", "fmul", "dmul", "idiv", "ldiv",
    "fdiv", "ddiv", "irem", "lrem", "frem", "drem", "ineg", "lneg", "fneg",
    "dneg", "ishl", "lshl", "ishr", "lshr", "iushr", "lushr", "iand", "land",
    "ior", "lor", "ixor", "lxor", "iinc", "i2l", "i2f", "i2d", "l2i", "l2f",
    "l2d", "f2i", "f2l", "f2d", "d2i", "d2l", "d2f", "i2b", "i2c", "i2s", "lcmp",
    "fcmpl", "fcmpg", "dcmpl", "dcmpg", "ifeq", "ifne", "iflt", "ifge","ifgt",
    "ifle", "if_icmpeq", "if_icmpne", "if_icmplt", "if_icmpge", "if_icmpgt",
    "if_icmple", "if_acmpeq", "if_acmpne", "goto", "jsr", "ret", "tableswitch",
    "lookupswitch", "ireturn", "lreturn", "freturn", "dreturn", "areturn",
    "return", "getstatic", "putstatic", "getfield", "putfield", "invokevirtual",
    "invokespecial", "invokestatic", "invokeinterface", "invokedynamic",
    "new", "newarray", "anewarray", "arraylength", "athrow", "checkcast",
    "instanceof", "monitorenter", "monitorexit", "wide", "multianewarray",
    "ifnull", "ifnonnull", "goto_w", "jsr_w", "breakpoint", 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "impdep1", "impdep2"
};