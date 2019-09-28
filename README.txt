Projetos de Sistemas Distribuídos

Versao : 01
Membros: 
	Francisco de Almeida Araújo 45701,
	Nuno Burnay 46406,
	Robin Vassantlal 46408.

Notas: 
	1. Para cada subdivisao eh necessario um computador diferente.
	2. Deve ser usado o porto 50000 em todos os casos.

Como executar?

Para executar ir até ao diretório onde o makefile se encontra e escrever o comando make.
Antes de executar os passos seguintes eh necessario fazer o make mesmo que os executaveis ja
estajam criados.
 	1. Activar/Funcionamento/Desligar o Servidor Secundario
		Ainda do mesmo diretório escrever ./server seguido do tamanho da lista e do Porto, de modo
		a que o servidor esteja à espera de clientes.
		Para ver o que se encontra na tabela basta escrever print
		No lado do servidor para se fechar basta fazer CTRL-C.
 	2. Activar/Funcionamento/Desativar o Servidor Primario
		No mesmo diretório escrever ./server seguido do tamanho da lista,do porto e do IP:Porto do secundario, de modo
		a que o servidor esteja à espera de clientes.
		Para ver o que se encontra na tabela basta escrever print
		No lado do servidor para se fechar basta fazer CTRL-C.
	3. Activacao/Funcionamento dos Clientes
		Em outro terminal ,mas no mesmo diretório, escrever ./client seguido do ip e do porto do servidor primário e ip e porto 
		do servidor secundário separados com ':'.
		Pode estar mais do que um cliente de cada vez no servidor mas o total não pode passar de 4 elementos 
		(pode ser alterado no ficheiro table_server.c no define N_OF_FDS).
		De seguida, no terminal do cliente, escrever o comando que queremos executar.
		Esse comando (que pode ser: put/2, get/1,update/2,del/1,size/0) são todos escritos em minúsculas.
		Em caso de erro do envio da mensagem do lado do cliente (por a mensagem simplesmente não ter chegado ou o 
		servidor não se encontra presente).
		O cliente fica a espera RETRY_TIME (defina no ficheior network_client-private.h) e tenta no máximo mais uma vez 
		estabelecer ligação e tenta enviar tudo de novo, mas no caso em que os servidores forem abaixo e voltrem a iniciar já não vai ter dados presentes. 
		Para fechar o cliente é necessário apenas escrever quit (em minúsculo).
	4. Reiniciamento dos Servidores
		Para reiniciar qualquer um dos servidores basta inicialo como se inicia o Servidor Secundario (referido na nota 1)