import socket
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext

SERVER_IP = "127.0.0.1"
SERVER_PORT = 54182


def conectar_e_enviar(modo):
    """
    Função genérica para enviar código.
    modo: "MODE:RUN" ou "MODE:DL"
    """
    codigo_fonte = txt_codigo.get("1.0", tk.END)
    if not codigo_fonte.strip():
        messagebox.showwarning("Aviso", "Código vazio!")
        return None

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((SERVER_IP, SERVER_PORT))

        # 1. Enviar cabeçalho + código
        mensagem = modo + "\n" + codigo_fonte
        client_socket.sendall(mensagem.encode("utf-8"))
        client_socket.shutdown(socket.SHUT_WR)

        # 2. Receber resposta (Binária ou Texto)
        dados_recebidos = b""
        while True:
            parte = client_socket.recv(4096)
            if not parte:
                break
            dados_recebidos += parte

        client_socket.close()
        return dados_recebidos

    except Exception as e:
        messagebox.showerror("Erro", f"Erro na conexão: {str(e)}")
        return None


def acao_executar():
    # Solicita execução e recebe texto
    resposta_bytes = conectar_e_enviar("MODE:RUN")

    if resposta_bytes:
        texto = resposta_bytes.decode("utf-8", errors="ignore")
        txt_saida.config(state=tk.NORMAL)
        txt_saida.delete("1.0", tk.END)
        txt_saida.insert(tk.INSERT, texto)
        txt_saida.config(state=tk.DISABLED)


def acao_baixar():
    # Solicita download e recebe binário
    dados_binarios = conectar_e_enviar("MODE:DL")

    if dados_binarios:
        # Verifica se o servidor retornou erro de compilação (texto) em vez de binário
        # Uma checagem simples: arquivos ELF (Linux Executable) começam com .ELF
        # Mas se for erro, será texto legível.
        if dados_binarios.startswith(b"[ERRO"):
            messagebox.showerror(
                "Erro de Compilação",
                "O servidor não gerou o binário pois houve erro no código.\nVeja a saída no console.",
            )
            # Mostra o erro na tela de saída também
            txt_saida.config(state=tk.NORMAL)
            txt_saida.delete("1.0", tk.END)
            txt_saida.insert(tk.INSERT, dados_binarios.decode("utf-8"))
            txt_saida.config(state=tk.DISABLED)
            return

        # Abre diálogo para salvar arquivo
        caminho_arquivo = filedialog.asksaveasfilename(
            defaultextension="",
            title="Salvar Executável Linux",
            filetypes=[("Executável", "*"), ("Todos os arquivos", "*.*")],
        )

        if caminho_arquivo:
            try:
                with open(caminho_arquivo, "wb") as f:
                    f.write(dados_binarios)
                messagebox.showinfo(
                    "Sucesso",
                    f"Arquivo salvo em:\n{caminho_arquivo}\n\nNota: Para rodar no Linux local, use: chmod +x <arquivo>",
                )
            except Exception as e:
                messagebox.showerror("Erro ao Salvar", str(e))


# --- Interface Gráfica ---
janela = tk.Tk()
janela.title("Cliente C - Compilador Remoto")
janela.geometry("600x550")

lbl_codigo = tk.Label(janela, text="Editor de Código (C):", font=("Arial", 10, "bold"))
lbl_codigo.pack(pady=5)

txt_codigo = scrolledtext.ScrolledText(janela, height=15)
txt_codigo.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)
txt_codigo.insert(
    tk.INSERT,
    '#include <stdio.h>\n\nint main() {\n    printf("Download funcionando!\\n");\n    return 0;\n}\n',
)

# Frame para botões lado a lado
frame_botoes = tk.Frame(janela)
frame_botoes.pack(pady=10)

# Botão Executar (Texto)
btn_run = tk.Button(
    frame_botoes,
    text="▶ Compilar e Executar",
    command=acao_executar,
    bg="green",
    fg="white",
    font=("Arial", 10, "bold"),
    width=20,
)
btn_run.pack(side=tk.LEFT, padx=10)

# Botão Baixar (Binário) - Requisito
btn_download = tk.Button(
    frame_botoes,
    text="⬇ Baixar Executável",
    command=acao_baixar,
    bg="blue",
    fg="white",
    font=("Arial", 10, "bold"),
    width=20,
)
btn_download.pack(side=tk.LEFT, padx=10)

lbl_saida = tk.Label(
    janela, text="Console de Saída / Erros:", font=("Arial", 10, "bold")
)
lbl_saida.pack(pady=5)

txt_saida = scrolledtext.ScrolledText(janela, height=10, bg="#f0f0f0")
txt_saida.config(state=tk.DISABLED)
txt_saida.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

janela.mainloop()
