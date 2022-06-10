# Designing-a-Virtual-Memory-Manager
Designing a Virtual Memory Manager with 128 frames, FIFO and LRU

Esta repositorio é a implmentação do problema do livro Operating System Concepts, Silberschatz, A. et al, 10a, página P-51. Ela conta com as seguintes alterções no problema: A implementação deverá ser aquela em que a memória física tem apenas 128 frames; 

O programa deve ser implementado em C e ser executável em sistemas Linux, Unix ou macOS, com a compilação feita Makefile, através simplesmente do comando make via terminal, e retornar o arquivo com nome vm executável;

Os frames na memória física devem ser preenchido do 0 ao 127, e quando a memória estiver cheia, aplicasse o algoritmo de substituição a fim de identificar qual frame será atualizado;

Deve-se implementar dois algoritmos de substituição de página, a saber fifo e lru, e dois
para substituição da TLB, também fifo e lru;

O primeiro argumento por linha de comando será um arquivo de endereços lógicos (similar ao
address.txt anexado ao Classroom), o segundo argumento será o tipo de algoritmo a ser
utilizado para substituição da página (fifo ou lru), e o terceiro argumento o algoritmo a ser
utilizado na substituição da TLB (fifo ou lru). Por exemplo, a chamada:
```./vm address.txt lru fifo```
indica que o algoritmo de substituição da página será o lru e da TLB o fifo.

A busca pela referência na TLB deverá ser realizada com uma thread para cada entrada da
TLB a fim de simular uma otimização da busca;

O arquivo de saída será denominado como correct.txt;

Esta implementação foi desenvolvida no ambiente linux.

# Como clonar

Para rodar esta implementação é preciso dar gitclone neste repositorio, por meio do seguinte comando:
```$ git clone https://github.com/joaoguilherme2003/Designing-a-Virtual-Memory-Manager.git```

# Compilar
```c
$ make
```
# Rodar
```c
$ ./vm (arquivos com os endereços .txt) (metodo substituição frames) (método substituição tlb)
```
# Clean binary
```c
$ make clean
```

