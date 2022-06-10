#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

const int TAM_PAGE = 256;
const int TAM_TLB = 16;
const int QUANT_FRAMES = 128;

FILE *arquivo, *arquivo2, *arquivo3;

int tabela_pag[256];  // pageid
int tlb[16][2];       // pageid e frame
signed char buffer[256];
int memoria_fisica[128][256];
pthread_t tlb_threads[16];
int lru_frames[128];         // index = frame, conteudo = tempo que foi utilizado
int tlb_frames_lru[16][2];          // 0 = frame, 1 = tempo que foi utilizado

long long page_id; 
int check_tlb = 0, tlb_hits = 0, frame; 
int frame_index = 0, index_thread = 0, index_tlb = 0;

void lerBackingstore();
void *buscarTLB(void *arg);
void atualizaLRU();
void atualizaLRUTLB();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{ 
    if (strcmp("fifo", argv[2]) && strcmp("lru", argv[2])) {            // Tratamento de Erro
        printf("Formato Errado!\n");
        return 0;
    }
    if (strcmp("fifo", argv[3]) && strcmp("lru", argv[3])) {
        printf("Formato Errado!\n");
        return 0;
    }
    
    memset(tabela_pag, -1, sizeof(tabela_pag));                        // Inicializa as estruturas de array com um valor especifico
    for (int i = 0; i < QUANT_FRAMES; i++) {
        lru_frames[i] = 1024;    
    }
    for (int i = 0; i < TAM_TLB; i++) {
        tlb_frames_lru[i][1] = 1024;    
    }
    
    for (int i = 0; i < QUANT_FRAMES; i++) {
        memoria_fisica[i][0] = -1;
    }
    for (int i = 0; i < TAM_TLB; i++) {
        tlb[i][0] = -1;
    }
    
    arquivo = fopen(argv[1], "r");
    arquivo2 = fopen("correct.txt", "w");
    arquivo3 = fopen("BACKING_STORE.bin", "rb");
    
    if (arquivo == NULL) {
  		printf("Arquivo não encontrado\n");
  		return 0;
	}

    int endereco;
    long long offset;
    int endereco_logico, enderecos_traduzidos = 0, j = 0, endereco_fisico = 0, page_faults = 0;
    int valor = 0, check_pagetable = 0, frame_menos_ocorrido = 0, maior = -1, index_maior = 0; 

    while ((fscanf(arquivo, "%d", &endereco) != EOF)) {
        
        endereco_logico = endereco;     // Tradução de endereços
          
        offset = endereco;
  		offset = offset & 255;
    
        page_id = endereco;           
  		page_id = page_id >> 8;
  		page_id = page_id & 255;
  		
        enderecos_traduzidos++;
  
        check_tlb = 0, check_pagetable = 0, index_thread = 0; 
      
        for (int i = 0; i < TAM_TLB; i++) {                            // Busca na TBL por meio de threads
            pthread_create(&(tlb_threads[i]), NULL, buscarTLB, NULL);
        }                                 
        for (int i = 0; i < TAM_TLB; i++) {
            pthread_join(tlb_threads[i], NULL);
        }
        
        if (check_tlb == 0) {                   // Caso nao encontrado no TLB
            
            for (int i = 0; i < TAM_PAGE; i++) {       // Procura na tabela de paginas
                if (tabela_pag[i] == page_id) { 
                    
                    frame = i;
                    check_pagetable = 1;
      		    break;
                }
            }
            if (check_pagetable == 0 && !strcmp(argv[2], "fifo")) {         // FIFO) caso o frame não seja encontrado na pagetable é preciso ir para o BACKINGSTORE
                
                page_faults++;                                  
                frame = frame_index;
                frame_index++;                
                if (frame_index == 128) {                      // Caso cheio, volta ao começo, tirando os primeiros
                    frame_index = 0;
                }
                lerBackingstore();           
            }
            else if (check_pagetable == 0 && !strcmp(argv[2], "lru")) {           // LRU) caso o frame não seja encontrado na pagetable é preciso ir para o BACKINGSTORE
                //printf("a\n");
                page_faults++;
                maior = -1;
                for (int i = 0; i < QUANT_FRAMES; i++) {
                    if (lru_frames[i] > maior) {            // Busca o mais velho
                        maior = lru_frames[i];
                        index_maior = i;
                    }
                }
                frame = index_maior;                        // Substitui pelo mais velho
                lerBackingstore();
            }

            if (!strcmp(argv[3], "fifo")) {
                
                tlb[index_tlb][0] = page_id;            // Atualizar o TLB FIFO
                tlb[index_tlb][1] = frame;
                index_tlb++;
                if (index_tlb == 16) {                    // Caso cheio, volta ao começo, tirando os primeiros
                    index_tlb = 0;
                }
            }
            else if (!strcmp(argv[3], "lru")) {
               
                maior = -1, j = 0;
                for (int i = 0; i < TAM_TLB; i++) {
                    if (tlb_frames_lru[i][1] > maior) {
                        maior = tlb_frames_lru[i][1];
                        index_maior = i;
                    }  
                }
                tlb[index_maior][0] = page_id;                // Substitui pelo mais velho
                tlb[index_maior][1] = frame;
            }
        }
        //printf("%d\n", maior);
        if (!strcmp(argv[2], "lru")) {
            atualizaLRU();
        }
        if (!strcmp(argv[3], "lru")) {
            atualizaLRUTLB();
        }
        tabela_pag[frame] = page_id;                              // salva na page table
        endereco_fisico = frame * 256 + offset;                   // calcula endereco fisico
        valor = memoria_fisica[frame][offset];                    // Obtem o Value 
        fprintf(arquivo2, "Virtual address: %d Physical address: %d Value: %d\n", endereco_logico, endereco_fisico, valor);

    }
    double tblrate = (double) tlb_hits / enderecos_traduzidos;
    double pagerate = (double) page_faults / enderecos_traduzidos;
    fprintf(arquivo2 ,"Number of Translated Addresses = %d\nPage Faults = %d\nPage Fault Rate = %.3f\nTLB Hits = %d\nTLB Hit Rate = %.3f", 
    enderecos_traduzidos, page_faults, pagerate, tlb_hits, tblrate);
    fclose(arquivo);
    fclose(arquivo2);
    fclose(arquivo3);

    return 0;
}

void lerBackingstore()
{
    fseek(arquivo3, page_id * 256, SEEK_SET);       
    if (fread(buffer, sizeof(signed char), 256, arquivo3))  // if adicionado para evitar um warning
    for (int i = 0; i < 256; i++) {
        memoria_fisica[frame][i] = buffer[i];  // Salva na memoria fisica
    }
}

void *buscarTLB(void *arg) 
{
    pthread_mutex_lock(&mutex);
    if (tlb[index_thread][0] == page_id) {
        tlb_hits++;
        frame = tlb[index_thread][1];
        check_tlb = 1;
    }
    index_thread++;
    pthread_mutex_unlock(&mutex); 
    return arg;
}
void atualizaLRU() 
{
    for (int i = 0; i < QUANT_FRAMES; i++) {    
        if (i == frame) {
            lru_frames[i] = 0;        // O frame que acabou de ser usado é zerado
        }
        else if (lru_frames[i] != 1024) {
            lru_frames[i]++;          // Os demais frames "envelhecem"
        }
    }
}
void atualizaLRUTLB() 
{
    for (int i = 0; i < TAM_TLB; i++) {    
        tlb_frames_lru[i][0] = tlb[i][1];        // Igualo o frame tlb lru com o tlb atual
        if (tlb[i][1] == frame) {
            tlb_frames_lru[i][1] = 0;            // A idade do frame adicionado é igualado a 0
        }
        else if (tlb_frames_lru[i][1] != 1024) {
            tlb_frames_lru[i][1]++;              // os demais envelhecem
        }
    }
}
