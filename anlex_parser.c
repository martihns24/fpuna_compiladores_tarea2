/*
 *	Analizador Léxico	
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Práctica de Programación Nro. 1
 *	
 *	Descripcion:
 *	Implementa un analizador léxico que reconoce números, identificadores, 
 * 	palabras reservadas, operadores y signos de puntuación para un lenguaje
 * 	con sintaxis tipo Pascal.
 *	
 */

/*********** Inclusión de cabecera **************/
#include "anlex.h"


/************* Variables globales **************/

int consumir;			/* 1 indica al analizador lexico que debe devolver
						el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];		// string utilizado para cargar mensajes de error
token t;				// token global para recibir componentes del Analizador Lexico

// variables para el analizador lexico

FILE *archivo;			// Fuente pascal
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char id[TAMLEX];		// Utilizado por el analizador lexico
int delantero=-1;		// Utilizado por el analizador lexico
int fin=0;				// Utilizado por el analizador lexico
int numLinea=1;			// Numero de Linea
int cantidadError=0;	// Cantidad de errores
char conjuntosPrimero[11][5][13];//conjuntos primeros de la gramatica dada
char conjuntosSegundo[11][3][13];//Conjuntos segundos de la gramatica dada
typedef int bool;		// Tipo de Datos booleanos
enum { false, true };	// Tipos booleanos

/**************** Funciones **********************/


// Rutinas del analizador lexico

void error(const char* mensaje)
{
	cantidadError = cantidadError + 1;
	printf("\nLin %d: Error Lexico. %s.",numLinea,mensaje);	
}

void sigLex()
{
	int i=0;
	char c=0;
	int acepto=0;
	int estado=0;
	char msg[41];
	entrada e;

	while((c=fgetc(archivo))!=EOF)
	{
		
		if (c==' ' || c=='\t')
			continue;	//eliminar espacios en blanco
		else if(c=='\n')
		{
			//incrementar el numero de linea
			numLinea++;
			continue;
		}
		else if (isdigit(c))
		{
				//es un numero
				i=0;
				estado=0;
				acepto=0;
				id[i]=c;
				
				while(!acepto)
				{
					switch(estado){
					case 0: //una secuencia netamente de digitos, puede ocurrir . o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=0;
						}
						else if(c=='.'){
							id[++i]=c;
							estado=1;
						}
						else if(tolower(c)=='e'){
							id[++i]=c;
							estado=3;
						}
						else{
							estado=6;
						}
						break;
					
					case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
						c=fgetc(archivo);						
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(c=='.')
						{
							i--;
							fseek(archivo,-1,SEEK_CUR);
							estado=6;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 2://la fraccion decimal, pueden seguir los digitos o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(tolower(c)=='e')
						{
							id[++i]=c;
							estado=3;
						}
						else
							estado=6;
						break;
					case 3://una e, puede seguir +, - o una secuencia de digitos
						c=fgetc(archivo);
						if (c=='+' || c=='-')
						{
							id[++i]=c;
							estado=4;
						}
						else if(isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 4://necesariamente debe venir por lo menos un digito
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 5://una secuencia de digitos correspondiente al exponente
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							estado=6;
						}break;
					case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
						if (c!=EOF)
							ungetc(c,archivo);
						else
							c=0;
						id[++i]='\0';
						acepto=1;
						t.pe=buscar(id);
						if (t.pe->compLex==-1)
						{
							strcpy(e.lexema,id);
							e.compLex=NUM;
							strcpy(e.complex_cadena,"LITERAL_NUM");
							insertar(e);
							t.pe=buscar(id);
						}
						t.compLex=NUM;
						break;
					case -1:
						if (c==EOF)
							error("No se esperaba el fin de archivo");
						else
							error(msg);
						exit(1);
					}
				}
			break;
		}
		else if (c==':')
		{
			//puede ser un : o un operador de asignacion
			c=fgetc(archivo);
			if (c=='='){
				t.compLex=OPASIGNA;
				t.pe=buscar(":=");
			}
			else{
				ungetc(c,archivo);
				t.compLex=':';
				t.pe=buscar(":");
			}
			break;
		}
		else if (c==',')
		{
			t.compLex=',';
			t.pe=buscar(",");
			break;
		}
		else if (c=='[')
		{
			t.compLex='[';
			t.pe=buscar("[");
			break;
		}
		else if (c==']')
		{
			t.compLex=']';
			t.pe=buscar("]");
			break;
		}
		else if (c=='{')
		{
			t.compLex='{';
			t.pe=buscar("{");
			break;
		}
		else if (c=='}')
		{
			t.compLex='}';
			t.pe=buscar("}");
			break;
		}
		else if (c=='\"')
		{//un caracter o una cadena de caracteres
			i=0;
			id[i]=c;
			i++;
			do{
				c=fgetc(archivo);
				if (c=='\"')
				{
					c=fgetc(archivo);
					if (c=='\"')
					{
						id[i]=c;
						i++;
						id[i]=c;
						i++;
					}
					else
					{
						id[i]='\"';
						i++;
						break;
					}
				}
				else if(c==EOF)
				{
					error("Se llego al fin de archivo sin finalizar un literal");
				}
				else{
					id[i]=c;
					i++;
				}
			}while(isascii(c));
			id[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			if (t.pe->compLex==-1)
			{
				strcpy(e.lexema,id);
				if (strlen(id)==3 || strcmp(id,"''''")==0)
					e.compLex=CAR;
				else
					e.compLex=LITERAL;
					strcpy(e.complex_cadena,"LITERAL_CADENA");
				insertar(e);
				t.pe=buscar(id);
				t.compLex=e.compLex;
			}
			break;
		}
		else if (c!=EOF)
		{
			sprintf(msg,"%c no esperado",c);
			error(msg);
		}
	}
	if (c==EOF)
	{
		t.compLex=EOF;
		t.pe=&e;
	}
	
}

void element()
{
	if (strcmp(t.pe->complex_cadena,"L_CORCHETE") == 0)
	{
		match("L_CORCHETE");
		tagname();
		if(strcmp("COMA",t.pe->complex_cadena)==0)
		{
			match("COMA");
			if (strcmp("L_LLAVE",t.pe->complex_cadena)==0)
			{
				attributes();
				match("COMA");
				if(strcmp("L_CORCHETE",t.pe->complex_cadena)==0)
				{
					element_list();	
				}
				else if (strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0)
				{
					element_list();
				}
			}
			if(strcmp("L_CORCHETE",t.pe->complex_cadena)==0)
			{
				element_list();
			}
			else if (strcmp("LITERAL_CADENA",t.pe->complex_cadena) == 0)
			{
				element_list();
			}
		}
		match("R_CORCHETE");
		
	}
	else if (strcmp(t.pe->complex_cadena,"LITERAL_CADENA") == 0){
		match("LITERAL_CADENA");
	}
	else{
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(1,t.pe->complex_cadena);
	
	}
	
}

void tagname(){
		if (strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
			match("LITERAL_CADENA");
		}
		else{
			cantidadError = cantidadError + 1;
			printf("Error Sintactico\n");
			panicModeConSincronizacion(2,t.pe->complex_cadena);
		}
}

void attributes(){
	if (strcmp("L_LLAVE",t.pe->complex_cadena)==0){
		match("L_LLAVE");
		if (strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
			attributes_list();
		}
		match("R_LLAVE");
		
	}
	else{
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(3,t.pe->complex_cadena);
	}
}

void attributes_list(){
	if(strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
		attribute();
		a();
	}
	else{
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(4,t.pe->complex_cadena);
	}
}

void a(){
	if(strcmp("COMA",t.pe->complex_cadena)==0){
		match("COMA");
		attribute();
		a();
	}
	else if (strcmp("R_LLAVE",t.pe->complex_cadena)==0){
		//nada
	}else{
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(5,t.pe->complex_cadena);
	}	
}

void attribute(){
	if (strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
		attribute_name();
		match("DOS_PUNTOS");
		attribute_value();
	}
	else {
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(6,t.pe->complex_cadena);
	}
}

void attribute_name(){
	if(strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
		match("LITERAL_CADENA");
	}else {
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(7,t.pe->complex_cadena);
	}
}

void attribute_value(){
	if(strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
		match("LITERAL_CADENA");
	}
	else if(strcmp("LITERAL_NUM",t.pe->complex_cadena)==0){
		match("LITERAL_NUM");
	}
	else if(strcmp("PR_TRUE",t.pe->complex_cadena)==0){
		match("PR_TRUE");
	}
	else if(strcmp("PR_FALSE",t.pe->complex_cadena)==0){
		match("PR_FALSE");
	}
	else if(strcmp("PR_NULL",t.pe->complex_cadena)==0){
		match("PR_NULL");
	}
	else {
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(8,t.pe->complex_cadena);
	}	
}	


void element_list(){
	if(strcmp("L_CORCHETE",t.pe->complex_cadena)==0 || strcmp("LITERAL_CADENA",t.pe->complex_cadena)==0){
		element();
		e();
	}
	else {
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(9,t.pe->complex_cadena);
	}
}	

void e(){
	if (strcmp("COMA",t.pe->complex_cadena)==0){
		match("COMA");
		element();
		e();
	}
	else if (strcmp("R_CORCHETE",t.pe->complex_cadena)==0){
		//nada
	}
	else{
		cantidadError = cantidadError + 1;
		printf("Error Sintactico\n");
		panicModeConSincronizacion(10,t.pe->complex_cadena);
	}
}

void match(char* proximoToken){
	if (strcmp(t.pe->complex_cadena,proximoToken) == 0){
		sigLex();	
	}
	else{
		cantidadError = cantidadError + 1;
		printf("Error no se esperaba: %s \n",proximoToken);
	}
	
}

void initConjuntosPrimero(){
	int i=0;
	//carga los conjuntos primeros en la matriz
	/*	0:jsonml
		1:element
		2:tag_name
		3:attributes
		4:attributes_list
		5:a
		6:attribute
		7:attribute_name
		8:attribute_value
		9:element_list
		10:e
	*/
	
		strcpy(conjuntosPrimero[1][0],"L_CORCHETE");
		strcpy(conjuntosPrimero[1][1],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[2][0],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[3][0],"L_LLAVE");
		strcpy(conjuntosPrimero[4][0],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[5][0],"COMA");
		strcpy(conjuntosPrimero[5][1],"EMPTY");
		strcpy(conjuntosPrimero[6][0],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[7][0],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[8][0],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[8][1],"LITERAL_NUM");
		strcpy(conjuntosPrimero[8][2],"PR_TRUE");
		strcpy(conjuntosPrimero[8][3],"PR_FALSE");
		strcpy(conjuntosPrimero[8][4],"PR_NULL");
		strcpy(conjuntosPrimero[9][0],"L_CORCHETE");
		strcpy(conjuntosPrimero[9][1],"LITERAL_CADENA");
		strcpy(conjuntosPrimero[10][0],"COMA");
		strcpy(conjuntosPrimero[10][1],"EMPTY");
}

void initConjuntosSegundo(){
	int i=0;
	//carga los conjuntos segundos en la matriz
	/*	0:jsonml
		1:element
		2:tag_name
		3:attributes
		4:attributes_list
		5:a
		6:attribute
		7:attribute_name
		8:attribute_value
		9:element_list
		10:e
	*/
		
	strcpy(conjuntosSegundo[1][0],"EOF");
	strcpy(conjuntosSegundo[1][1],"COMA");
	strcpy(conjuntosSegundo[1][2],"R_CORCHETE");
	strcpy(conjuntosSegundo[2][0],"COMA");
	strcpy(conjuntosSegundo[2][1],"R_CORCHETE");
	strcpy(conjuntosSegundo[3][0],"COMA");
	strcpy(conjuntosSegundo[3][1],"R_CORCHETE");
	strcpy(conjuntosSegundo[4][0],"R_LLAVE");
	strcpy(conjuntosSegundo[5][0],"R_LLAVE");
	strcpy(conjuntosSegundo[6][0],"COMA");
	strcpy(conjuntosSegundo[6][1],"R_LLAVE");
	strcpy(conjuntosSegundo[7][0],"DOS_PUNTOS");
	strcpy(conjuntosSegundo[8][0],"COMA");
	strcpy(conjuntosSegundo[8][1],"R_LLAVE");
	strcpy(conjuntosSegundo[9][0],"R_CORCHETE");
	strcpy(conjuntosSegundo[10][0],"R_CORCHETE");
}

bool existeTokenPrimero(int produccion, char* tokenActual){
	
	int i=0;
	for(i=0; i<5; i++){
		//busca el token actual en la matriz de conjunto primero
		if(strcmp(tokenActual,conjuntosPrimero[produccion][i])==0){
				return true;
		}
	}
	return false;
}

bool existeTokenSegundo(int produccion, char* tokenActual){
	
	int i=0;
	for(i=0; i<3; i++){
		//busca el token actual en la matriz de conjunto segundo
		if(strcmp(tokenActual,conjuntosSegundo[produccion][i])==0){
				return true;
		}
	}
	return false;
}	

void panicModeConSincronizacion(int produccion, char *tokenActual){
	
	bool existeSegundo = existeTokenSegundo(produccion,tokenActual);
	
	if(!existeSegundo){
		bool existePrimero = existeTokenPrimero(produccion,tokenActual);
		if (!existePrimero){
			scan();
		}
	}else if(existeSegundo || strcmp(tokenActual,"EOF")==0){
		pop();
	}
}

void scan(){
	printf("Hizo SCAN\n");
	sigLex();
}

void pop(){
	printf("Hizo POP\n");
	//genera una produccion con EMPTY
}

int main(int argc,char* args[])
{
	// inicializar analizador lexico

	initTabla();
	initTablaSimbolos();
	initConjuntosPrimero();
	initConjuntosSegundo();
	
	if(argc > 1)
	{
		if (!(archivo=fopen(args[1],"rt"))){
			printf("Archivo no encontrado.\n");
			exit(1);
		}	
		sigLex();
		element();
		int i=0;
		int j=0;
		
		if(cantidadError>0){
			printf("Se ha encontrado errores en el fuente\n");
		}else {
			printf("El fuente es sintacticamente correcto\n");
		}
		fclose(archivo);
	}else{
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}

	return 0;
}
