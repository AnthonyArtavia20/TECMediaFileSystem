#Desde aquí se va a iniciar la interfaz que llamará la lógica de c++ y sus métodos
import tkinter as tk

from PIL import ImageTk, Image
from tkinter import ttk, filedialog, messagebox
from datetime import datetime

class TecMFSApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("TEC Media File System")
        self.geometry("800x600")
        self.configure(bg="#f2f2f2")
        self.logoAndTitlePanel()
        self.create_disk_panel()
        self.create_file_table()
        self.create_controls()

    def logoAndTitlePanel(self):
        information_frame = tk.Frame(self, bg="#f2f2f2", pady=8)
        information_frame.pack(fill=tk.X, padx=10)

        #Sub-frame izquierdo para logo + titulo
        left_frame = tk.Frame(information_frame, bg="#f2f2f2")
        left_frame.pack(side=tk.LEFT, anchor="nw")

        # Titulo
        tk.Label(information_frame, text="TEC Media File System App", font=("Arial", 15, "bold"), bg="#f2f2f2").pack(anchor="w")

        #logo
        image1Logo = Image.open("GUI/gui/assets/logo.png")
        image1Logo = image1Logo.resize((110,110), Image.LANCZOS)
        self.logo_image = ImageTk.PhotoImage(image1Logo)
        logo_label = tk.Label(information_frame, image=self.logo_image, bg="#f2f2f2")
        logo_label.pack(side=tk.LEFT, padx=(0, 20))
        logo_label.pack(anchor="w")

        #Sub-frame derecho para estado del RAID
        right_frame = tk.Frame(information_frame, bg="#f2f2f2")
        right_frame.pack(side=tk.RIGHT, anchor="ne")

        self.estado_label = tk.Label(right_frame, text="Estado: RAID OK", font=("Arial", 11), bg="#f2f2f2", fg="green")
        self.estado_label.pack(anchor="e", padx=10, pady=5)


    def create_disk_panel(self):
        disk_frame = tk.Frame(self, bg="#f2f2f2", pady=10)
        disk_frame.pack(fill=tk.X)
        for i in range(4):
            frame = tk.Frame(disk_frame, bg="#d9d9d9", bd=1, relief=tk.RIDGE, padx=10, pady=5)
            frame.pack(side=tk.LEFT, expand=True, padx=5)
            tk.Label(frame, text=f"Disco {i+1}", font=("Arial", 10, "bold")).pack()
            tk.Label(frame, text="🟢 OK", font=("Arial", 12)).pack()
            tk.Label(frame, text="Uso: 75%", font=("Arial", 10)).pack()

    def create_file_table(self):
        table_frame = tk.Frame(self)
        table_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        tittle_label = tk.Label(table_frame, text="Archivos disponibles en el Raid", font=("Arial", 12, "bold"), anchor="w")
        tittle_label.pack(anchor="w", padx=5, pady=(0,5)) #Aliniado a la izquierda con espaciado

        self.file_table = ttk.Treeview(table_frame, columns=("fileName","size", "date", "actions"), show="headings")
        self.file_table.heading("fileName", text="Nombre de Archivo")
        self.file_table.heading("size", text="Tamaño")
        self.file_table.heading("date", text="Fecha")
        self.file_table.heading("actions", text="Acciones")
        self.file_table.column("fileName", width=100, anchor=tk.CENTER)
        self.file_table.column("size", width=100, anchor=tk.CENTER)
        self.file_table.column("date", width=150, anchor=tk.CENTER)
        self.file_table.column("actions", width=150, anchor=tk.CENTER)
        self.file_table.pack(fill=tk.BOTH, expand=True)
        self.file_table.bind("<ButtonRelease-1>", self.on_table_click)
        self.populate_dummy_files()

    def populate_dummy_files(self):
        files = [("informe.pdf", "2.1 MB", "2025-06-04","Descargar | Eliminar"),
                 ("foto.jpg", "5.4 MB", "2025-06-03","Descargar | Eliminar"),
                 ("datos.txt", "0.4 MB", "2025-06-01","Descargar | Eliminar")]
        for name, size, date,actions in files:
            self.file_table.insert("", "end", values=(name, size, date, actions))

    def create_controls(self):
        control_frame = tk.Frame(self, pady=5)
        control_frame.pack(fill=tk.X, padx=10)
        tk.Label(control_frame, text="Buscar:").pack(side=tk.LEFT)
        self.search_entry = tk.Entry(control_frame, width=30)
        self.search_entry.pack(side=tk.LEFT, padx=5)
        tk.Button(control_frame, text="Buscar", command=self.search_file).pack(side=tk.LEFT, padx=5)
        tk.Button(control_frame, text="Subir archivo...", command=self.upload_file).pack(side=tk.RIGHT)

    def search_file(self):
        query = self.search_entry.get().lower()
        for item in self.file_table.get_children():
            filename = self.file_table.item(item)['values'][0].lower()
            if query in filename:
                self.file_table.selection_set(item)
                self.file_table.see(item)
                return
        messagebox.showinfo("Buscar", "Archivo no encontrado.")

    def upload_file(self):
        path = filedialog.askopenfilename()
        if path:
            filename = path.split("/")[-1]
            size = "1.2 MB"
            date = datetime.now().strftime("%Y-%m-%d")
            self.file_table.insert("", "end", values=(filename, size, date))
            messagebox.showinfo("Subida", f"Archivo '{filename}' subido (simulado).")

    def on_table_click(self,event):
        item = self.file_table.identify_row(event.y)
        column = self.file_table.identify_column(event.x)

        if item and column == "#4": #columna "Acciones"
            filename = self.file_table.item(item)["values"][0]
            self.show_action_dialog(filename)

    def show_action_dialog(self, filename):
        # Crear ventana emergente personalizada
        dialog = tk.Toplevel(self)
        dialog.title("Seleccionar acción")
        dialog.geometry("300x120")
        dialog.grab_set()  # Bloquea interacción con la ventana principal

        tk.Label(dialog, text=f"¿Qué desea hacer con '{filename}'?", font=("Arial", 10)).pack(pady=10)

        button_frame = tk.Frame(dialog)
        button_frame.pack(pady=5)

        tk.Button(button_frame, text="Descargar", width=12, command=lambda: self.descargar_archivo(dialog, filename)).pack(side=tk.LEFT, padx=10)

        tk.Button(button_frame, text="Eliminar", width=12, command=lambda: self.eliminar_archivo(dialog, filename)).pack(side=tk.RIGHT, padx=10)

    def descargar_archivo(self, dialog, filename):
        dialog.destroy()
        print(f"Descargando '{filename}'... (simulado)")
        messagebox.showinfo("Descargar", f"Archivo '{filename}' descargado correctamente (simulado)")

    def eliminar_archivo(self, dialog, filename):
        dialog.destroy()
        print(f"Eliminando '{filename}'... (simulado)")
        messagebox.showwarning("Eliminar", f"Archivo '{filename}' eliminado (simulado)")
