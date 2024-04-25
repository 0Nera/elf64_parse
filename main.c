#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define EI_MAG0     0
#define ELFMAG0     0x7f
#define EI_MAG1     1
#define ELFMAG1     'E'
#define EI_MAG2     2
#define ELFMAG2     'L' 
#define EI_MAG3     3  
#define ELFMAG3     'F'
#define SHT_SYMTAB	2

typedef uint64_t elf64_addr_t;   // Адрес
typedef uint64_t elf64_offset_t; // Смещение
typedef uint64_t elf64_xword_t;  // Целочисленное длинное слово без знака
typedef uint64_t elf64_sxword_t; // Целочисленное длинное слово с знаком
typedef uint32_t elf64_word_t;   // Целочисленное слово без знака
typedef uint32_t elf64_sword_t;  // Целочисленное слово с знаком
typedef uint16_t elf64_half_t;   // Среднее целое число без знака
typedef uint8_t elf64_small_t;   // Малое целое число без знака

typedef struct {
    elf64_small_t e_ident[16]; // Идентификация ELF
    elf64_half_t e_type;       // Тип объектного файла
    elf64_half_t e_machine;    // Тип компьютера
    elf64_word_t e_version;    // Версия объектного файла
    elf64_addr_t e_entry;      // Адрес точки входа
    elf64_offset_t e_phoff;    // Смещение заголовка программы
    elf64_offset_t e_shoff;    // Смещение заголовка раздела
    elf64_word_t e_flags;      // Флаги, зависящие от процессора
    elf64_half_t e_ehsize;     // Размер заголовка ELF
    elf64_half_t e_phentsize;  // Размер записи заголовка программы
    elf64_half_t e_phnum;      // Количество записей в заголовке программы
    elf64_half_t e_shentsize;  // Размер записи в заголовке раздела
    elf64_half_t e_shnum;      // Количество записей в заголовке раздела
    elf64_half_t e_shstrndx;   // Строковый табличный индекс названия раздела
} elf64_header_t;

typedef struct {
    elf64_word_t sh_name;       // Название раздела
    elf64_word_t sh_type;       // Тип раздела
    elf64_xword_t sh_flags;     // Атрибуты раздела
    elf64_addr_t sh_addr;       // Виртуальный адрес в памяти
    elf64_offset_t sh_offset;   // Смещение в файле
    elf64_xword_t sh_size;      // Размер раздела
    elf64_word_t sh_link;       // Ссылка на другой раздел
    elf64_word_t sh_info;       // Дополнительная информация
    elf64_xword_t sh_addralign; // Граница выравнивания адреса
    elf64_xword_t sh_entsize;   // Размер записей, если в разделе есть таблица
} elf64_section_header_t;

typedef struct {
    elf64_addr_t r_offset; // Адрес ссылки
    elf64_xword_t r_info;  // Индекс символа и тип перемещения
} elf64_rel_t;

typedef struct {
    elf64_addr_t r_offset;   // Адрес ссылки
    elf64_xword_t r_info;    // Индекс символа и тип перемещения
    elf64_sxword_t r_addend; // Постоянная часть выражения
} elf64_rela_t;

typedef struct {
    elf64_word_t p_type;     // Тип сегмента
    elf64_word_t p_flags;    // Атрибуты сегмента
    elf64_offset_t p_offset; // Смещение в файле
    elf64_addr_t p_vaddr;    // Виртуальный адрес в памяти
    elf64_addr_t p_paddr;    // Зарезервирован
    elf64_xword_t p_filesz;  // Размер сегмента в файле
    elf64_xword_t p_memsz;   // Размер сегмента в памяти
    elf64_xword_t p_align;   // Выравнивание сегмента
} elf64_phdr_t;

typedef struct {
    elf64_word_t st_name;   // Название символа
    elf64_small_t st_info;  // Тип и атрибуты привязки
    elf64_small_t st_other; // Зарезервировано
    elf64_half_t st_shndx;  // Индекс таблицы разделов
    elf64_addr_t st_value;  // Значение символа
    elf64_xword_t st_size;  // Размер объекта (например, общий)
} elf64_sym_t;

typedef struct {
    elf64_sxword_t d_tag; // Тип динамического элемента
    union {
        elf64_xword_t d_val; // Значение динамического элемента
        elf64_addr_t d_ptr;  // Указатель динамического элемента
    } d_un;
} elf64_dyn_t;

elf64_header_t *elf64_get_header(void *data) {
    return (elf64_header_t *)(data);
}

static inline elf64_section_header_t *elf64_sheader(elf64_header_t *hdr) {
    return (elf64_section_header_t *)((elf64_addr_t)hdr + hdr->e_shoff);
}

static inline elf64_section_header_t *elf64_section(elf64_header_t *hdr, elf64_offset_t idx) {
    return &elf64_sheader(hdr)[idx];
}

static inline char *elf64_str_table(elf64_header_t *hdr) {
    if (hdr->e_shstrndx == 0x0)
        return NULL;
    return (char *)hdr + elf64_section(hdr, hdr->e_shstrndx)->sh_offset;
}

static inline char *elf64_lookup_string(elf64_header_t *hdr, elf64_offset_t offset) {
    char *strtab = elf64_str_table(hdr);
    if (strtab == NULL)
        return NULL;
    return strtab + offset;
}

static elf64_sym_t *elf64_get_symval(elf64_header_t *hdr, elf64_offset_t table, elf64_offset_t idx) {
    if (table == 0 || idx == 0)
        return 0;
    elf64_section_header_t *symtab = elf64_section(hdr, table);

    uint32_t symtab_entries = symtab->sh_size / symtab->sh_entsize;
    if (idx >= symtab_entries) {
        printf("Symbol Index out of Range (%ld:%lu).\n", table, idx);
        return NULL;
    }

    uintptr_t symaddr = (uint64_t)hdr + symtab->sh_offset;
    return (elf64_sym_t *)&((elf64_sym_t *)symaddr)[idx];
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Use: elf64_parce <file>\n");
        return 0;
    }

    char *filename = argv[1];
    FILE *fp;

    printf("Parsing %s...\n", filename);

    fp = fopen(filename, "r");

    fseek(fp, 0, SEEK_END);
    uint64_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    void *fcontent = malloc(size);
    fread(fcontent, 1, size, fp);

    elf64_header_t *head = elf64_get_header(fcontent);
    elf64_section_header_t *symtab = NULL;

    if (head->e_ident[0] != ELFMAG0 ||
        head->e_ident[1] != ELFMAG1 ||
        head->e_ident[2] != ELFMAG2 ||
        head->e_ident[3] != ELFMAG3) {
        printf("Error: Invalid format\n");
        return -1;
    }

    printf("Entry: 0x%lx\n", head->e_entry);

    
    elf64_section_header_t *symtab_section = NULL;
    char *string_table = NULL;
    for (int i = 0; i < head->e_shnum; i++) {
        elf64_section_header_t *shdr = elf64_section(head, i);
        if (shdr->sh_type == SHT_SYMTAB) {
            symtab_section = shdr;
            elf64_section_header_t *strtab_section = elf64_section(head, shdr->sh_link);
            string_table = (char *)head + strtab_section->sh_offset;
            break;
        }
    }

    if (symtab_section && string_table) {
        printf("\nSymbol Table:\n");
        printf("%6s %8s %16s %sn", "Index", "Value", "Size", "Name");

        int num_symbols = symtab_section->sh_size / symtab_section->sh_entsize;
        for (int i = 0; i < num_symbols; i++) {
            elf64_sym_t *sym = elf64_get_symval(head, symtab_section - elf64_sheader(head), i);
            if (sym) {
                printf("%6d %08lx %16lx %s\n", i, sym->st_value, sym->st_size,
                    string_table + sym->st_name);
            }
        }
    } else {
        printf("Symbol table not found.\n");
    }


    fclose(fp);
    free(fcontent);

    return 0;
}