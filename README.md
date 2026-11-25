Sistema de Atendimento Clínico
Projeto desenvolvido em C para a avaliação de programação. O sistema permite cadastrar pacientes, gerar senhas, consultar a fila, chamar pacientes e finalizar atendimentos.
As senhas são divididas em três categorias: normal, preferencial e urgente. Os pacientes urgentes são chamados primeiro, seguidos dos preferenciais e dos normais. Dentro de cada categoria, é respeitada a ordem de chegada.
O código foi dividido em funções. lerPaciente faz a leitura e validação dos dados, classificarPaciente define a prioridade, gerarSenha cria as senhas, enfileirar e desenfileirar controlam as filas, chamarPaciente e finalizarAtendimento gerenciam o atendimento e exibirPainel mostra as filas.
Para compilar e executar, use o GCC no terminal:
gcc main.c -o clinica
./clinica
No Windows, execute .\clinica.exe após a compilação.
Exemplo: ao cadastrar as senhas N001, P001 e U001, a ordem de atendimento será U001, P001 e N001.
As informações ficam armazenadas apenas enquanto o programa está aberto.
