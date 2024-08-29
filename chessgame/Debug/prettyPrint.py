
class BitBoardPrinter:
    #Prints bit board given number. 
    #Assumes bit 0 starts at bottom right of the board
    def __init__(self, val):
        self.val = val

    def to_string(self):
        str=""
        for i in range(8):
            for j in range(8):
                str=str+ ("1" if self.val & (1 << (63-(8*i+j))) else "0")
            str=str+"\n"
        return str
        

def lookup_type(val):
    if str(val.type) == "long long unsigned int":
        return BitBoardPrinter(val)
    return None

gdb.pretty_printers.append(lookup_type)