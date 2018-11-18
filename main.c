#include "interpreter.h"

int main(int argc, char **argv) 
{
	char *className = argv[1];
	// se tiver 3 argumentos, executa somente o leitor exibidor
	// senão executa a jvm
	int  argumentsNumber = argc-2;
	char **offset = argv+2;

	// numero de argumentos incorreto
	if(argc == 1)
		printf("Uso:\tWIN: jvm.exe class_file.class t|a"
				 "\n\tLINUX: ./jvm class_file.class t|a\n");		
	// para executar a jvm
	else if(argc == 2)
		startJVM(className, argumentsNumber, NULL);
	// para executar o leitor exibidor
	else
		startJVM(className, argumentsNumber, offset);
	
	return 0;
}
