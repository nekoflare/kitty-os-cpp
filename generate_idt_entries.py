def generate_idt_entry(func_name, segment, gate_type, dpl, present, ist):
    return f"IDT_ENTRY(&{func_name}, {segment}, {gate_type}, {dpl}, {present}, {ist})"

def generate_idt_entries(num_entries):
    entries = []
    for i in range(num_entries):
        entry = generate_idt_entry(f"idt_isr{i}", "IDT_DEFAULT_SEGMENT", "IDT_GATE_TYPE_INTERRUPT", "IDT_DPL_RING_0", "IDT_PRESENT", "IDT_NO_IST")
        entries.append(entry)
    return entries

def generate_externs(num_entries):
    externs = []
    for i in range(num_entries):
        externs.append(f"extern \"C\" void idt_isr{i}();")
    return externs

def generate_idt_program(num_entries):
    header = ("//\n"
              "// Created by Piotr on 03.05.2024.\n"
              "//\n"
              "\n"
              "#include <kernel/hal/x64/idt/idt.hpp>\n"
              "\n")

    externs = generate_externs(num_entries)
    extern_declarations = "\n".join(externs)

    idt_entries = generate_idt_entries(num_entries)
    idt_code = f"volatile GateDescriptor64 idt[{num_entries}] = {{\n        " + ",\n        ".join(idt_entries) + "\n};"

    return header + extern_declarations + "\n\n" + idt_code

def main():
    num_entries = 256
    idt_program = generate_idt_program(num_entries)
    print(idt_program)

if __name__ == "__main__":
    main()
