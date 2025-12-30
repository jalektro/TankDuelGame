import socket
import tkinter as tk
from tkinter import ttk

PYNQ_IP = "192.168.0.113"
P1_PORT = 5001
P2_PORT = 5002

# Bit positions
MOVEFORWARD = 1 << 0
MOVEBACK    = 1 << 1
MOVELEFT    = 1 << 2
MOVERIGHT   = 1 << 3
GUNLEFT     = 1 << 4
GUNRIGHT    = 1 << 5
SHOOT       = 1 << 6

class UDPControllerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("UDP Game Controller")
        self.root.geometry("700x850")
        
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Current player (1 or 2)
        self.current_player = 1
        
        # Create UI
        self.create_widgets()
        
        # Bind keyboard events
        self.root.bind('<KeyPress>', self.on_key_press)
        self.root.bind('<KeyRelease>', self.on_key_release)
        
        # Track pressed keys
        self.pressed_keys = set()
        
    def create_widgets(self):
        # IP Address configuration
        ip_frame = ttk.LabelFrame(self.root, text="PYNQ Board IP Address", padding=10)
        ip_frame.pack(pady=10, padx=10, fill=tk.X)
        
        tk.Label(ip_frame, text="IP Address:").pack(side=tk.LEFT, padx=5)
        self.ip_entry = tk.Entry(ip_frame, width=15)
        self.ip_entry.insert(0, PYNQ_IP)
        self.ip_entry.pack(side=tk.LEFT, padx=5)
        
        ttk.Button(ip_frame, text="Update IP", command=self.update_ip).pack(side=tk.LEFT, padx=5)
        
        self.ip_status = tk.Label(ip_frame, text="", fg="green")
        self.ip_status.pack(side=tk.LEFT, padx=10)
        
        # Store current IP
        self.current_ip = PYNQ_IP
        
        # Player and keyboard selection
        player_frame = ttk.LabelFrame(self.root, text="Player Selection", padding=10)
        player_frame.pack(pady=10, padx=10, fill=tk.X)
        
        self.player_var = tk.IntVar(value=1)
        ttk.Radiobutton(player_frame, text="Player 1 (Port 5001)", 
                       variable=self.player_var, value=1,
                       command=self.update_player).pack(side=tk.LEFT, padx=10)
        ttk.Radiobutton(player_frame, text="Player 2 (Port 5002)", 
                       variable=self.player_var, value=2,
                       command=self.update_player).pack(side=tk.LEFT, padx=10)
        
        # Keyboard layout selection
        keyboard_frame = ttk.LabelFrame(self.root, text="Keyboard Layout", padding=10)
        keyboard_frame.pack(pady=10, padx=10, fill=tk.X)
        
        self.keyboard_var = tk.StringVar(value="AZERTY")
        ttk.Radiobutton(keyboard_frame, text="AZERTY", 
                       variable=self.keyboard_var, value="AZERTY",
                       command=self.update_labels).pack(side=tk.LEFT, padx=10)
        ttk.Radiobutton(keyboard_frame, text="QWERTY", 
                       variable=self.keyboard_var, value="QWERTY",
                       command=self.update_labels).pack(side=tk.LEFT, padx=10)
        
        # Status display
        status_frame = ttk.LabelFrame(self.root, text="Current Command Status", padding=10)
        status_frame.pack(pady=10, padx=10, fill=tk.BOTH, expand=True)
        
        # Command indicators
        self.indicators = {}
        self.command_labels = {}
        
        commands = [
            ("MOVEFORWARD", "green"),
            ("MOVEBACK", "blue"),
            ("MOVELEFT", "yellow"),
            ("MOVERIGHT", "orange"),
            ("GUNLEFT", "purple"),
            ("GUNRIGHT", "cyan"),
            ("SHOOT", "red")
        ]
        
        for i, (cmd, color) in enumerate(commands):
            frame = tk.Frame(status_frame)
            frame.pack(pady=5, fill=tk.X)
            
            label = tk.Label(frame, text="", width=25, anchor='w')
            label.pack(side=tk.LEFT)
            self.command_labels[cmd] = label
            
            indicator = tk.Canvas(frame, width=30, height=30, bg='gray', highlightthickness=1)
            indicator.pack(side=tk.LEFT, padx=10)
            self.indicators[cmd] = (indicator, color)
        
        # Last sent info
        info_frame = ttk.LabelFrame(self.root, text="Last Sent", padding=10)
        info_frame.pack(pady=10, padx=10, fill=tk.X)
        
        self.info_label = tk.Label(info_frame, text="No data sent yet", 
                                   font=("Courier", 10), anchor='w')
        self.info_label.pack(fill=tk.X)
        
        # Instructions
        instr_frame = ttk.LabelFrame(self.root, text="Controls", padding=10)
        instr_frame.pack(pady=10, padx=10, fill=tk.X)
        
        self.instr_label = tk.Label(instr_frame, text="")
        self.instr_label.pack()
        
        # Now update all labels after all widgets are created
        self.update_labels()
    
    def update_labels(self):
        """Update UI labels based on keyboard layout"""
        layout = self.keyboard_var.get()
        
        if layout == "AZERTY":
            labels = {
                "MOVEFORWARD": "Move Forward (Z)",
                "MOVEBACK": "Move Back (S)",
                "MOVELEFT": "Move Left (Q)",
                "MOVERIGHT": "Move Right (D)",
                "GUNLEFT": "Gun Left (A)",
                "GUNRIGHT": "Gun Right (E)",
                "SHOOT": "Shoot (Space)"
            }
        else:  # QWERTY
            labels = {
                "MOVEFORWARD": "Move Forward (W)",
                "MOVEBACK": "Move Back (S)",
                "MOVELEFT": "Move Left (A)",
                "MOVERIGHT": "Move Right (D)",
                "GUNLEFT": "Gun Left (Q)",
                "GUNRIGHT": "Gun Right (E)",
                "SHOOT": "Shoot (Space)"
            }
        
        for cmd, label_text in labels.items():
            self.command_labels[cmd].config(text=label_text)
        
        self.update_instructions()
    
    def update_instructions(self):
        """Update instruction text based on keyboard layout"""
        layout = self.keyboard_var.get()
        
        if layout == "AZERTY":
            instructions = "Z/S: Forward/Back | Q/D: Left/Right | A/E: Gun Left/Right | Space: Shoot"
        else:  # QWERTY
            instructions = "W/S: Forward/Back | A/D: Left/Right | Q/E: Gun Left/Right | Space: Shoot"
        
        self.instr_label.config(text=instructions)
        
    def update_player(self):
        self.current_player = self.player_var.get()
        self.info_label.config(text=f"Switched to Player {self.current_player}")
    
    def update_ip(self):
        new_ip = self.ip_entry.get().strip()
        if new_ip:
            self.current_ip = new_ip
            self.ip_status.config(text=f"✓ Using {new_ip}", fg="green")
            self.info_label.config(text=f"IP updated to {new_ip}")
        else:
            self.ip_status.config(text="✗ Invalid IP", fg="red")
        
    def on_key_press(self, event):
        key = event.keysym.lower()
        # Map physical keys based on keyboard layout
        mapped_key = self.map_key(key)
        if mapped_key:
            self.pressed_keys.add(mapped_key)
            self.send_command()
    
    def on_key_release(self, event):
        key = event.keysym.lower()
        mapped_key = self.map_key(key)
        if mapped_key and mapped_key in self.pressed_keys:
            self.pressed_keys.remove(mapped_key)
            self.send_command()
    
    def map_key(self, physical_key):
        """Map physical keyboard key to game command key"""
        layout = self.keyboard_var.get()
        
        if layout == "AZERTY":
            # AZERTY physical keys -> QWERTY game logic
            key_map = {
                'z': 'w',  # Z on AZERTY = W forward
                's': 's',  # S stays same
                'q': 'a',  # Q on AZERTY = A left
                'd': 'd',  # D stays same
                'a': 'q',  # A on AZERTY = Q gun left
                'e': 'e',  # E stays same
                'space': 'space'
            }
            return key_map.get(physical_key)
        else:  # QWERTY
            # Direct mapping for QWERTY
            if physical_key in ['w', 's', 'a', 'd', 'q', 'e', 'space']:
                return physical_key
        return None
    
    def send_command(self):
        bits = 0
        active_commands = []
        
        for key in self.pressed_keys:
            if key == "w":
                bits |= MOVEFORWARD
                active_commands.append("MOVEFORWARD")
            elif key == "s":
                bits |= MOVEBACK
                active_commands.append("MOVEBACK")
            elif key == "a":
                bits |= MOVELEFT
                active_commands.append("MOVELEFT")
            elif key == "d":
                bits |= MOVERIGHT
                active_commands.append("MOVERIGHT")
            elif key == "q":
                bits |= GUNLEFT
                active_commands.append("GUNLEFT")
            elif key == "e":
                bits |= GUNRIGHT
                active_commands.append("GUNRIGHT")
            elif key == "space":
                bits |= SHOOT
                active_commands.append("SHOOT")
        
        # Send UDP packet
        port = P1_PORT if self.current_player == 1 else P2_PORT
        self.sock.sendto(bytes([bits]), (self.current_ip, port))
        
        # Update indicators
        self.update_indicators(active_commands)
        
        # Update info label
        binary_str = format(bits, '08b')
        hex_str = format(bits, '02X')
        info = f"Player {self.current_player} | Port: {port} | Bits: 0x{hex_str} ({binary_str}) | Active: {', '.join(active_commands) if active_commands else 'None'}"
        self.info_label.config(text=info)
    
    def update_indicators(self, active_commands):
        for cmd, (indicator, color) in self.indicators.items():
            if cmd in active_commands:
                indicator.config(bg=color)
            else:
                indicator.config(bg='gray')
    
    def on_closing(self):
        self.sock.close()
        self.root.destroy()

def main():
    root = tk.Tk()
    app = UDPControllerGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()

if __name__ == "__main__":
    main()