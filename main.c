#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#define CAPACIDADE 100
#define TOTAL_CATEGORIAS 3

/* A ordem das categorias também representa a prioridade de chamada. */
typedef enum { URGENTE, PREFERENCIAL, NORMAL } Categoria;

typedef struct {
    char genero;
    int idade;
    int gestante;
    int lactante;
    int necessidadeEspecial;
    int obesidade;
} Paciente;

typedef struct {
    Categoria categoria;
    unsigned long long numero;
} Senha;

/* Fila circular: permite reutilizar os espaços após cada chamada. */
typedef struct {
    Senha itens[CAPACIDADE];
    int inicio;
    int quantidade;
    unsigned long long ultimoNumero;
} Fila;

typedef struct {
    Fila filas[TOTAL_CATEGORIAS];
    Senha atual;
    int emAtendimento;
} Sistema;

/* Retorna 1 para linha válida, 0 para EOF e -1 para linha longa. */
int lerLinha(char *texto, int tamanho) {
    int c;
    if (fgets(texto, tamanho, stdin) == NULL) return 0;
    if (strchr(texto, '\n') != NULL) return 1;
    c = getchar();
    if (c == EOF || c == '\n') return 1;
    while ((c = getchar()) != '\n' && c != EOF) { }
    return -1;
}

/* Valida todo o texto, inclusive caracteres após o número. */
int lerInteiro(const char *pergunta, int minimo, int maximo, int *valor) {
    char texto[128], *fim;
    long numero;
    int estado;
    while (1) {
        printf("%s", pergunta);
        fflush(stdout);
        estado = lerLinha(texto, sizeof texto);
        if (estado == 0) return 0;
        if (estado == 1) {
            errno = 0;
            numero = strtol(texto, &fim, 10);
            if (fim != texto) {
                while (isspace((unsigned char)*fim)) fim++;
                if (*fim == '\0' && errno == 0 && numero >= minimo && numero <= maximo) {
                    *valor = (int)numero;
                    return 1;
                }
            }
        }
        printf("Entrada invalida. Informe um inteiro de %d a %d.\n", minimo, maximo);
    }
}

int lerGenero(char *genero) {
    int opcao;
    if (!lerInteiro("Genero (1 - Feminino, 2 - Masculino, 3 - Outro/nao informado): ", 1, 3, &opcao)) return 0;
    *genero = opcao == 1 ? 'F' : (opcao == 2 ? 'M' : 'O');
    return 1;
}

int lerSimNao(const char *pergunta, int *resposta) {
    return lerInteiro(pergunta, 0, 1, resposta);
}

int lerPaciente(Paciente *p) {
    p->gestante = 0;
    if (!lerGenero(&p->genero)) return 0;
    if (!lerInteiro("Idade (0 a 130): ", 0, 130, &p->idade)) return 0;
    /* Para esta atividade, a pergunta de gestação é omitida no gênero masculino. */
    if (p->genero != 'M' && !lerSimNao("Gestante? (1 - Sim, 0 - Nao): ", &p->gestante)) return 0;
    if (!lerSimNao("Lactante? (1 - Sim, 0 - Nao): ", &p->lactante)) return 0;
    if (!lerSimNao("Necessidade especial? (1 - Sim, 0 - Nao): ", &p->necessidadeEspecial)) return 0;
    if (!lerSimNao("Obesidade? (1 - Sim, 0 - Nao): ", &p->obesidade)) return 0;
    return 1;
}

Categoria classificarPaciente(Paciente p) {
    if (p.idade > 80) return URGENTE;
    if (p.gestante || p.lactante || p.necessidadeEspecial || p.obesidade ||
        (p.genero == 'F' && p.idade >= 60) ||
        (p.genero == 'M' && p.idade >= 65)) return PREFERENCIAL;
    return NORMAL;
}

const char *nomeCategoria(Categoria categoria) {
    switch (categoria) {
        case URGENTE: return "Urgente";
        case PREFERENCIAL: return "Preferencial";
        default: return "Normal";
    }
}

void exibirSenha(Senha senha) {
    const char prefixos[] = {'U', 'P', 'N'};
    printf("%c%03llu", prefixos[senha.categoria], senha.numero);
}

Senha gerarSenha(Fila *fila, Categoria categoria) {
    Senha senha;
    senha.categoria = categoria;
    senha.numero = ++fila->ultimoNumero;
    return senha;
}

int enfileirar(Fila *fila, Senha senha) {
    int fim;
    if (fila->quantidade == CAPACIDADE) return 0;
    fim = (fila->inicio + fila->quantidade) % CAPACIDADE;
    fila->itens[fim] = senha;
    fila->quantidade++;
    return 1;
}

int desenfileirar(Fila *fila, Senha *senha) {
    if (fila->quantidade == 0) return 0;
    *senha = fila->itens[fila->inicio];
    fila->inicio = (fila->inicio + 1) % CAPACIDADE;
    fila->quantidade--;
    return 1;
}

int cadastrarPaciente(Sistema *sistema) {
    Paciente paciente;
    Categoria categoria;
    Senha senha;
    Fila *fila;
    puts("\n--- CADASTRO DE PACIENTE ---");
    if (!lerPaciente(&paciente)) return 0;
    categoria = classificarPaciente(paciente);
    fila = &sistema->filas[categoria];
    if (fila->quantidade == CAPACIDADE || fila->ultimoNumero == ULLONG_MAX) {
        puts("Nao foi possivel emitir senha: limite da categoria atingido.");
        return 1;
    }
    senha = gerarSenha(fila, categoria);
    enfileirar(fila, senha);
    printf("Senha emitida: ");
    exibirSenha(senha);
    printf(" (%s)\n", nomeCategoria(categoria));
    return 1;
}

/* Procura a primeira fila não vazia na ordem U, P, N. */
int localizarProximaFila(const Sistema *sistema) {
    for (int i = 0; i < TOTAL_CATEGORIAS; i++) {
        if (sistema->filas[i].quantidade > 0) return i;
    }
    return -1;
}

void consultarProximaSenha(const Sistema *sistema) {
    int indice = localizarProximaFila(sistema);
    if (indice == -1) {
        puts("Nenhum paciente aguardando.");
        return;
    }
    const Fila *fila = &sistema->filas[indice];
    printf("Proxima senha: ");
    exibirSenha(fila->itens[fila->inicio]);
    printf(" (%s)\n", nomeCategoria((Categoria)indice));
}

void chamarPaciente(Sistema *sistema) {
    int indice;
    if (sistema->emAtendimento) {
        puts("Finalize o atendimento atual antes de chamar outro paciente.");
        return;
    }
    indice = localizarProximaFila(sistema);
    if (indice == -1) {
        puts("Nenhum paciente aguardando.");
        return;
    }
    if (desenfileirar(&sistema->filas[indice], &sistema->atual)) {
        sistema->emAtendimento = 1;
        printf("Senha chamada: ");
        exibirSenha(sistema->atual);
        printf(" (%s)\n", nomeCategoria(sistema->atual.categoria));
    }
}

void finalizarAtendimento(Sistema *sistema) {
    if (!sistema->emAtendimento) {
        puts("Nao ha atendimento em andamento.");
        return;
    }
    printf("Atendimento finalizado: ");
    exibirSenha(sistema->atual);
    putchar('\n');
    sistema->emAtendimento = 0;
}

void exibirPainel(const Sistema *sistema) {
    int total = 0;
    puts("\n========== PAINEL DE CHAMADA ==========");
    printf("Em atendimento: ");
    if (sistema->emAtendimento) {
        exibirSenha(sistema->atual);
        printf(" (%s)\n", nomeCategoria(sistema->atual.categoria));
    } else {
        puts("Nenhuma senha.");
    }
    consultarProximaSenha(sistema);
    puts("\nFilas de espera (ordem de prioridade):");
    for (int i = 0; i < TOTAL_CATEGORIAS; i++) {
        const Fila *fila = &sistema->filas[i];
        total += fila->quantidade;
        printf("%s (%d): ", nomeCategoria((Categoria)i), fila->quantidade);
        if (fila->quantidade == 0) printf("vazia");
        for (int j = 0; j < fila->quantidade; j++) {
            if (j > 0) printf(" -> ");
            exibirSenha(fila->itens[(fila->inicio + j) % CAPACIDADE]);
        }
        putchar('\n');
    }
    printf("Total em espera: %d\n", total);
    puts("======================================");
}

void exibirMenu(void) {
    puts("\n=== SISTEMA CLINICO ===");
    puts("1 - Cadastrar paciente");
    puts("2 - Consultar proxima senha");
    puts("3 - Chamar paciente");
    puts("4 - Finalizar atendimento");
    puts("5 - Exibir painel");
    puts("0 - Sair");
}

int main(void) {
    Sistema sistema = {0};
    int opcao;
    do {
        exibirMenu();
        if (!lerInteiro("Escolha uma opcao: ", 0, 5, &opcao)) break;
        switch (opcao) {
            case 1:
                if (!cadastrarPaciente(&sistema)) opcao = 0;
                break;
            case 2: consultarProximaSenha(&sistema); break;
            case 3: chamarPaciente(&sistema); break;
            case 4: finalizarAtendimento(&sistema); break;
            case 5: exibirPainel(&sistema); break;
            case 0: break;
        }
    } while (opcao != 0);
    puts("Sistema encerrado.");
    return 0;
}
