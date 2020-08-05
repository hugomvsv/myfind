#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>

#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/semaphore.h>
#include <mach/task.h>

#define N 4
#define NProds 2
#define NCons 6

typedef int (*PARAM)(struct dirent *entry, char *value);

typedef struct arg {
    PARAM opt;
    char *value;
} ARG;
typedef struct thread_data {
    pthread_t pthread_id;
    char *path;
    ARG args[50];
    int n_args;
    int nthrread;
    int tids[20];
    int *vecres[20];
    int nresultados;
    char pathsol[20][1000];
} T_DATA;
typedef struct thread_args {
    T_DATA *t_data;
    char *filename;
} TARGS;
char pathinicial[100];
char paths[30][1000];
int npaths = 0;
int prodptr = 0, consptr = 0;
int iteracao = 0;
pthread_mutex_t trinco_p = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trinco_c = PTHREAD_MUTEX_INITIALIZER;

static const char *semNameProd = "semPodeProd";
semaphore_t semPodeProd;

static const char *semNameCons = "semPodeCons";
semaphore_t semPodeCons;

int name_prefix(struct dirent *dir, char *value);

void lower_string(char *s);

int name(struct dirent *dir, char *value);

int name_subfix(struct dirent *dir, char *value);

int iname(struct dirent *dir, char *value);

int iname_prefix(struct dirent *dir, char *value);

int iname_subfix(struct dirent *dir, char *value);

int type(struct dirent *dir, char *value);

int empty(struct dirent *dir, char *value);

int executable(struct dirent *dir, char *value);

int mmin(struct dirent *dir, char *value);

int size(struct dirent *dir, char *value);

void parser(const char *args[], int nArgs, T_DATA *t_data);

void print_thread_data(T_DATA *t_data);

void lower_string(char *s);

int is_dir(char *file_path);

int is_dir(char *file_path);

void sniffDir(T_DATA *t_data, char *path);

void *consumidor(void *param);

void *produtor(void *param);

int main(int argc, const char *argv[]) {
    int b;
    T_DATA *t_data = (T_DATA *) malloc(sizeof(T_DATA));
    parser(argv, argc, t_data);
    //  print_thread_data(t_data);
    T_DATA th_data_array_prod;
    T_DATA *th_data_array_cons[NCons];
    pthread_t thread;
    pthread_t cons[N];


    semaphore_create(mach_task_self(), &semPodeProd, SYNC_POLICY_FIFO, N);
    semaphore_create(mach_task_self(), &semPodeCons, SYNC_POLICY_FIFO, 0);


    pthread_create(&thread, NULL, (void *) &produtor, t_data);
    pthread_join(thread, NULL);

    for (int i = 0; i < N; ++i) {

        pthread_create(&cons[i], NULL, (void *) &consumidor, t_data);
        pthread_join(cons[i], NULL);
    }


printf("Soluções\n");
    printf("__________________\n");
    for (int j = 0; j < t_data->nresultados; ++j) {
        printf("%s\n",t_data->pathsol[j]);
    }



    return 0;
}

void *produtor(void *param) {
    // semaphore_wait(semPodeProd);
    printf("----->>>>%d\n", iteracao);
    T_DATA *t_data = (T_DATA *) param;
    struct dirent *entry;
    DIR *dir;
    char *fullpath = (char *) malloc(256);
    int cont = 0;
    if (iteracao == 0) {
        static int k = 0;

        char *path = t_data->path;

        if (*(path + 0) == '.' &&
            *(path + 1) != '\0') //caso tenha ./worksheet2 por exemplo (está neste directório na pasta worksheet)
        {
            printf("\n entrei na primeira!");
            getcwd(fullpath, 256);
            printf("Tenho ponto e o meu full path é: %s\n", fullpath);
            memmove(&*(path + 0), &*(path + 1), strlen(path)); //remove o ponto do path
            strcat(fullpath, path);  //junta o path com o full path
            printf("Tenho ponto e o meu full path é: %s\n", fullpath);
            strcpy(pathinicial,fullpath);
        } else {
            if (*(path + 0) == '.') //caso tenho só . varre este directório completo
            {
                printf("entrei na segunda !");
                getcwd(fullpath, 256);
                printf("Tenho ponto e o meu full path é: %s\n", fullpath);
                memmove(&*(path + 0), &*(path + 1), strlen(path)); //remove o ponto do path
                strcat(fullpath, path);  //junta o path com o full path
                printf("Tenho ponto e o meu full path é: %s\n", fullpath);
                strcpy(pathinicial,fullpath);

            }
        }

       strcpy(paths[npaths],pathinicial);
        npaths++;
    } else if (iteracao != 0) {

        if ((dir = opendir(paths[iteracao - 1])) == NULL) //tenta abrir o diretório passado
            perror("opendir()error"); //caso não encontre o diretorio
        else {
            //puts("\nConteudo dentro do Dir passado:");
            while ((entry = readdir(dir)) != NULL) //começa a ler todos os dirs dentro do  root passado
            {
                cont = 0;
                if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) //ignora os '..' e '.'
                    continue;

                // printf("------------\nnome do dir: %s\n", entry->d_name);
                char *dirpath = (char *) malloc(256);
                char barra[] = "/";
                strcat(dirpath, paths[iteracao - 1]);
                strcat(dirpath, barra);
                strcat(dirpath, entry->d_name);


                if (is_dir(dirpath) == 1) {

                    strcpy(paths[npaths], dirpath);
                    npaths++;

                }


            }
            semaphore_signal(semPodeCons);        // aumenta o semaforo de consumidores
            closedir(dir);
        }
    }
    iteracao++;
    if (iteracao <= npaths)
        produtor(t_data);

    return 0;
}

void *consumidor(void *param) {

    T_DATA *t_data = (T_DATA *) param;
    while (1) {
        if(consptr==npaths)
            return 0;
        char *item;
        semaphore_wait(semPodeCons);        // espera por ser sina;iozado para poder consumir
        pthread_mutex_lock(&trinco_c);        // aguarda o lock da regiao critica
        item = paths[consptr];            // retira o itam do buf na posicao consptr

        int posicao = consptr;

        consptr++;       // coloca o consptr para a proxima posicao

        printf("-->%s\n",item);
        sniffDir(t_data, item);            // imprime
        pthread_mutex_unlock(
                &trinco_c);    // liberta o lock para outras threads consumidoras entrarem na regiao critica
        // semaphore_signal(semPodeProd);        // aumenta o semaforo de consumidores

    }


}


void
sniffDir(T_DATA *t_data, char *path) { //apenas o main thread entra nesta funçao para verificar se descodificar o path
    int cont = 0;             //conta o numero de ocorrencias que satisfaz o argumento
    DIR *dir;
    struct dirent *entry;
    char *fullpath = (char *) malloc(256);

    pthread_t thread;
    char pathsol[256];
    //senao tiver o path tem de ser completo
    //caso o path tenha . quer dizer que é neste directorio entao usamos o gtcwd e concatnamos
    //senao entrar em nenhum dos if's temos o path já completo

    if ((dir = opendir(path)) == NULL) //tenta abrir o diretório passado
        perror("opendir()error"); //caso não encontre o diretorio

    else {
        //printf("-->%s\n",path);
        while ((entry = readdir(dir)) != NULL) //começa a ler todos os dirs dentro do  root passado
        {
            cont = 0;
            if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) //ignora os '..' e '.'
                continue;

           // printf("------------\nnome do dir: %s\n", entry->d_name);
            for (int i = 0; i < (t_data->n_args); i++) {
                char *dirpath = (char *) malloc(256);      //para guardar o path total do diretorio
                char barra[] = "/";
                strcat(dirpath, fullpath);
                strcat(dirpath, barra);
                strcat(dirpath, entry->d_name);
                // printf("\nvalor:%s",t_data->args[i].value);
                if (t_data->args[i].opt(entry, t_data->args[i].value) == 1) {
                    cont++;             //conta o numero de ocorrencias que satisfaz o argumento
                    t_data->vecres[0]++;
                }

            }
            if (cont ==
                t_data->n_args) {       //se o contador for igual ao nosso numero de arguentos signifca que temos um match
                t_data->nresultados++;
               char sol[100];
                char barra[] = "/";
               strcpy(sol,path);
               strcat(sol,barra);
               strcat(sol,entry->d_name);
                strcpy(t_data->pathsol[t_data->nresultados],sol);
                t_data->nresultados++;

            }
        }
        closedir(dir);
    }

}


/**
 * funcao que ira dar parse aos argumentos do my find
 * @param args argumentos do argv
 * @param start onde começar
 * @param options a opcao do find
 * @param search o que pesquisar
 */
void parser(const char *args[], int nArgs, T_DATA *t_data) {

    if (nArgs > 1) {
        nArgs = nArgs - 1; //retira o nome do programa e o search
        t_data->path = (char *) malloc(strlen(args[1]));
        strcpy(t_data->path, args[1]);
        char *auxname;

        for (int i = 2; i <= nArgs; i++) {
            if (strcmp("-name", args[i]) == 0) {
                if (*(args[i + 1] + 0) == '*')//sufixo  tem ponto no inicio
                {
                    printf("\nSou sufixo");
                    auxname = (char *) malloc(strlen(args[i + 1])); //aloca memoria para o auxname
                    strcpy(auxname, args[i + 1]);//passa para auxname o arg
                    memmove(&*(auxname + 0), &*(auxname + 1), strlen(auxname)); //retira o ponto do inicio
                    t_data->args[t_data->n_args].opt = (PARAM) name_subfix; //atribui a funcao
                    t_data->args[t_data->n_args].value = auxname;
                    t_data->n_args++;
                    i++;
                } else {
                    if (*(args[i + 1] + (strlen(args[i + 1]) - 1)) == '*')//prefixo
                    {
                        printf("\nSou prefixo");
                        auxname = (char *) malloc(strlen(args[i + 1])); //aloca memoria para o auxname
                        strcpy(auxname, args[i + 1]);//passa para auxname o arg
                        memmove(&*(auxname + (strlen(auxname) - 1)), "", 1); //retira o ponto do inicio
                        t_data->args[t_data->n_args].opt = (PARAM) name_prefix; //atribui a funcao
                        t_data->args[t_data->n_args].value = auxname;
                        t_data->n_args++;
                        i++;
                    } else {
                        printf("\nsou simples\n");
                        t_data->args[t_data->n_args].opt = (PARAM) name;
                        t_data->args[t_data->n_args].value = (char *) malloc(strlen(args[i + 1]));
                        strcpy(t_data->args[t_data->n_args].value, args[i + 1]);
                        t_data->n_args++;
                        i++;
                    }
                }
            } else {
                if (strcmp("-iname", args[i]) == 0) {
                    if (*(args[i + 1] + 0) == '*')//sufixo  tem ponto no inicio
                    {
                        printf("\nSou sufixo");
                        auxname = (char *) malloc(strlen(args[i + 1])); //aloca memoria para o auxname
                        strcpy(auxname, args[i + 1]);//passa para auxname o arg
                        memmove(&*(auxname + 0), &*(auxname + 1), strlen(auxname)); //retira o ponto do inicio
                        lower_string(auxname);
                        t_data->args[t_data->n_args].opt = (PARAM) iname_subfix; //atribui a funcao
                        t_data->args[t_data->n_args].value = auxname;
                        t_data->n_args++;
                        i++;
                    } else {
                        if (*(args[i + 1] + (strlen(args[i + 1]) - 1)) == '*')//prefixo
                        {
                            printf("\nSou prefixo");
                            auxname = (char *) malloc(strlen(args[i + 1])); //aloca memoria para o auxname
                            strcpy(auxname, args[i + 1]);//passa para auxname o arg
                            memmove(&*(auxname + (strlen(auxname) - 1)), "", 1); //retira o ponto do inicio
                            lower_string(auxname);
                            t_data->args[t_data->n_args].opt = (PARAM) iname_prefix; //atribui a funcao
                            t_data->args[t_data->n_args].value = auxname;
                            t_data->n_args++;
                            i++;
                        } else {//simples
                            printf("\nSou simples");
                            t_data->args[t_data->n_args].opt = (PARAM) iname;
                            t_data->args[t_data->n_args].value = (char *) malloc(strlen(args[i + 1]));
                            strcpy(t_data->args[t_data->n_args].value, args[i + 1]);
                            lower_string(t_data->args[t_data->n_args].value);
                            t_data->n_args++;
                            i++;
                        }
                    }

                } else {
                    if (strcmp("-type", args[i]) == 0) {
                        t_data->args[t_data->n_args].opt = (PARAM) type;
                        t_data->args[t_data->n_args].value = (char *) malloc(strlen(args[i + 1]));
                        strcpy(t_data->args[t_data->n_args].value, args[i + 1]);
                        t_data->n_args++;
                        i++;
                    } else {
                        if (strcmp("-empty", args[i]) == 0) {
                            printf("boas sou vazio !");
                            t_data->args[t_data->n_args].opt = (PARAM) empty;
                            t_data->args[t_data->n_args].value = (char *) malloc(strlen("TRUE") + 1);
                            strcpy(t_data->args[t_data->n_args].value, "TRUE");
                            t_data->n_args++;
                        } else {
                            if (strcmp("-executable", args[i]) == 0) {
                                t_data->args[t_data->n_args].opt = (PARAM) executable;
                                t_data->args[t_data->n_args].value = (char *) malloc(strlen("TRUE") + 1);
                                strcpy(t_data->args[t_data->n_args].value, "TRUE");
                                t_data->n_args++;
                            } else {
                                if (strcmp("-mmin", args[i]) == 0) {
                                    t_data->args[t_data->n_args].opt = (PARAM) mmin;
                                    t_data->args[t_data->n_args].value = (char *) malloc(strlen(args[i + 1]));
                                    strcpy(t_data->args[t_data->n_args].value, args[i + 1]);
                                    t_data->n_args++;
                                    i++;
                                } else {
                                    if (strcmp("-size", args[i]) == 0) {

                                        t_data->args[t_data->n_args].opt = (PARAM) size;
                                        t_data->args[t_data->n_args].value = malloc(strlen(args[i + 1]));
                                        strcpy(t_data->args[t_data->n_args].value, args[i + 1]);
                                        t_data->n_args++;
                                        i++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    printf("\nNumero de argumentos %d ", t_data->n_args + 1);

}


void print_thread_data(T_DATA *t_data) {
    int n = t_data->n_args;
    for (int i = 0; i < n; i++) {
        printf("\n%s", t_data->args[i].value);
    }
    printf("\n");
}

int name(struct dirent *dir, char *value) {
   // printf("\nNAME");
    if (strcmp(dir->d_name, value) == 0) {      // se for igual retorna 1
        return 1;
    }
    return 0;
}

int name_prefix(struct dirent *dir, char *value) {
  //  printf("\nNAME_PREFIX");
    int tam = strlen(value);        //guarda o tamanho do arg passado
  //  printf("\nTAM:%d", tam);
    int flag = 0;
    for (int i = 0; i < tam; i++) {
        if (dir->d_name[i] == value[i]) {
            flag = 1;
        } else {
            flag = 0;
            break;
        }
    }
    return flag;
}

int name_subfix(struct dirent *dir, char *value) {
  //  printf("\nNAME_SUBFIX");
    int flag = 0;
    int tam = strlen(value);
    int aux = strlen(dir->d_name);
    aux = aux - tam;
    for (int i = 0; i < tam; i++) {
        if (dir->d_name[i + aux] == value[i]) {
            flag = 1;
        } else {
            flag = 0;
            break;
        }
    }
    if (flag == 1) return 1;
    return 0;

}

int iname(struct dirent *dir, char *value) {
  //  printf("\nINAME");
    char *aux = (char *) malloc(strlen(dir->d_name));
    aux = dir->d_name;
    lower_string(aux);
    if (strcmp(aux, value) == 0) {
        return 1;
    }
    return 0;
}

int iname_prefix(struct dirent *dir, char *value) {
 //   printf("\nINAME_PREFIX");
    char *aux = (char *) malloc(strlen(dir->d_name));
    aux = dir->d_name;
    lower_string(aux);
    int tam = strlen(value);
    int flag = 0;
    for (int i = 0; i < tam; i++) {
        if (aux[i] == value[i]) {
            flag = 1;
        } else {
            flag = 0;
            break;
        }
    }
    if (flag == 1) return 1;
    return 0;
}

int iname_subfix(struct dirent *dir, char *value) {
 //   printf("\nINAME_SUBFIX");
    char *stringaux = (char *) malloc(strlen(dir->d_name));
    stringaux = dir->d_name;
    lower_string(stringaux);
    int flag = 0;
    int tam = strlen(value);
    int aux = strlen(stringaux);
    aux = aux - tam;
    for (int i = 0; i < tam; i++) {
        if (stringaux[i + aux] == value[i]) {
            flag = 1;
        } else {
            flag = 0;
            break;
        }
    }
    if (flag == 1) return 1;
    return 0;

}

int type(struct dirent *dir, char *value) {
 //   printf("\nTYPE");
    if (value[0] == 'd') { // caso seja pedido do tipo diretorio
        if (dir->d_type == DT_DIR) {
            return 1;
        }
    } else {
        if (value[0] == 'f')//caso seja pedido do tipo file
        {
            if (dir->d_type == DT_REG) {
                return 1;
            }
        }

    }
    return 0;
}

int empty(struct dirent *dir, char *value) {
   // printf("\nEMPTY");
    struct stat st;
    stat(dir->d_name, &st);
    printf("tamanho do ficheiro ou dir : %lld",
           st.st_size);        //utilização da stat para verificar o tamanho, se for zero retorna 1
    int size = st.st_size;
    if (size == 0) {
        return 1;
    }
    return 0;
}

int executable(struct dirent *dir, char *value) {
//    printf("\nEXECUTABLE");
    struct stat st;
    stat(dir->d_name, &st);
    if (st.st_mode & S_IXUSR && dir->d_type != DT_DIR) {
        printf("\nE executavel");
        return 1;
    }
    return 0;
}

int mmin(struct dirent *dir, char *value) {
    //printf("\nMMIN");
    int tempomax = atoi(value);     //passa de string para int
    struct stat st;
    stat(dir->d_name, &st);
    struct tm *timeinfo;
    time_t t1 = st.st_mtime;        //t1 recebe a datade modificaçao
    time_t t2;
    time(&t2);
    timeinfo = localtime(&t2);
    double time = difftime(t2, t1);     //faz a diferença entre a data  atual e data de alteraçao
    if (tempomax > time) {              //se for menor do que o esperado retorna 1
        return 1;
    }
    return 0;
}

int size(struct dirent *dir, char *value) {
 //   printf("\nSIZE");
    float num = atoi(value);
    num *= 1000000;         //passagem para bytes
    printf("\nvalor passado: %f", num);
    struct stat st;
    stat(dir->d_name, &st);
    float size = st.st_size;

    printf("\nsize: %f", size);
    if (value[0] == '+') {
        if (size > num) return 1;       //se o simbolo for + e se for maior retorna 1
        return 0;

    } else {
        if (size < num) return 1;       //se o simbolo for - e se for menor retorna 1
        return 0;
    }
}

int is_dir(char *file_path) {
    struct stat sb;
    if (stat(file_path, &sb) != -1) // Check the return value of stat
    {
        if (S_ISREG(sb.st_mode) != 0) {
            return 0;
        } else {
            return 1;
        }
    }
    return 0;
}

void lower_string(char *s) {
    int c = 0;

    while (s[c] != '\0') {
        if (s[c] >= 'A' && s[c] <= 'Z') {
            s[c] = s[c] + 32;
        }
        c++;
    }
}
