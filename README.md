# myfind
Myfind - Pesquisa de ficheiros numa hierarquia de diretórios

O comando find no UNIX é um utilitário de linha de comando para percorrer uma
hierarquia de diretórios. O comando find é usado para procurar e localizar a lista de
ficheiros e diretórios com base nas condições especificadas nos argumentos. 
O comando
suporta a pesquisa por ficheiros, pasta, nome, data de criação, data de modificação, dono
e permissões.

1. Esta etapa do trabalho implica programar o “myfind”. O programa “myfind”
deve aceitar os argumentos anteriormente indicados e efetuar a pesquisa de todas as
ocorrências. A pesquisa deve ser feita com recurso a tarefas. Devem usar
sincronização entre tarefas para garantir o bom funcionamento do “myfind”.

a.  Devem fazer o parse dos argumentos.

b. Devem ser criadas “n threads”, cada tarefa deve consumir um
diretório. Ao encontrar um novo directório a “thread” deve criar uma nova
tarefa para consumir esse novo directório. Quando todas as tarefas
concluírem a procura, a “main thread” deve saber quantas
correspondências cada tarefa satisfez.

c.  Devem ser criadas “n threads consumidoras”, e “1 thread
produtoras”. A tarefa produtora deve produzir diretórios para serem
consumidos pelas threads consumidoras. Quando uma tarefa consumidora
acaba a procura no diretório corrente deve consultar se existe mais
diretórios para consumir. Quando todas as tarefas concluírem a procura, a
“main thread” deve saber quantas correspondências cada tarefa satisfez.

Opções a considerar:

● -name: procura por um ficheiro com um nome específico.

● -iname: procura por um ficheiro com um nome específico ignorando maiúsculas
ou minúsculas.

● -type type: procura por um tipo específico.

● -empty: procura por ficheiros ou diretórios vazios.

● -executable: procura por ficheiros executáveis.

● -mmin -60: procura ficheiros modificados há n minutos.

● -size +5M: procura os ficheiros com mais ou menos que x tamanho (megas)
