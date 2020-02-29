#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//CATALIN BAJENARU 311CB
unsigned char *arena = NULL;
int n, ind = 0;

void intpoi(int x, int p)
{
    // functie care pune un integer in arena (in loc de a folosi cast)
    int ma = ((1 <<8) - 1 );
    arena[p] = x & ma;
    arena[p + 1] = (x & (ma << 8))>> 8;
    arena[p + 2] = (x & (ma << 16))>> 16;
    arena[p + 3] = (x & (ma << 24))>> 24;
}
int poiint(int p)
{
    //functie care ia din arena un integer (in loc de a folosi cast)
    int x = 0;
    x = x | arena[p];
    x = x | (arena[p + 1] << 8);
    x = x | (arena[p + 2] << 16);
    x = x | (arena[p + 3] << 24);
    return x;

}

void release(int indx)
{
    int ant = poiint(indx - 8);
    if(poiint(ant + 8))
        intpoi(poiint(indx - 12), ant);   //II PUNE CELUI ANTERIOR ADRESA VIITOARE CORECTA
    if(poiint(indx - 12))
        intpoi(ant, poiint(indx - 12) + 4);    // II PUNE CELUI VIITOR ADRESA ANTERIOARA CORECTA
    int i = indx - 12, k = indx + poiint(indx - 4);
    if(ind == indx - 12)
        ind = poiint(indx - 12);
    for(; i < k; i++)
        arena[i] = 0;

}

void setval(int indx, int size, int val)
{
    int i, b;
    while(1)
    {
        i = indx;
        b = i + poiint(indx - 4);
        for(; i < b; i++)
        {
            arena[i] = val;
            size--;
            if(!size)return;
        }

        indx = poiint(indx - 12) + 12;
        if(indx == 12)return;
    }
}

void initialize(int size)
{
    arena = (unsigned char*)calloc(size, sizeof(char));
    n = size;
}

void dump()
{
    int i, j;
    for(i = 0; i < n; i+=8)
    {
        printf("%08X\t",i);

        if(i + 16 <= n)
        {

            for(j = i; j < i+8; j++)
            {
                printf("%02X ", arena[j]);
            }
            i += 8;
            printf(" ");

            for(j = i; j < i+8; j++)
            {
                printf("%02X ", arena[j]);
            }
            printf("\n");

        }
        else
        {
            int m;
            if(i + 8 > n)m = n;
            else m = i + 8;
            for(j = i; j < m; j++)
            {
                printf("%02X ", arena[j]);
            }
            i += 8;
            m = n % m;
            if(m)printf(" ");
            for(j = i; j < i + m; j++)
            {
                printf("%02X ", arena[j]);
            }
            printf("\n");

        }
    }
}


int alloc(int size)
{
    if(n <= size)return 0;
    if(ind == 0 && poiint(8) == 0 && size + 12 <= n) // VERIFICA DACA SE POATE PUNE LA INCEPUTUL ARENEI
    {
        intpoi(size, 8);
        return 12;
    }
    else if(ind - 12 >= size)
    {
        // daca se poate aloca in spatele primului bloc
        intpoi(ind, 0);
        intpoi(size, 8);
        ind = 0;
        return 12;
    }
    else
    {
        int act = ind, ok = 1, ant ;
        while(ok)
        {
            ant = poiint(act + 4);
            if(act-(ant + 24+poiint(ant + 8)) >= size)
            {
                //il poate insera intre anteriorul si actualul
                intpoi((ant + 12 + poiint(ant + 8)), act+4);
                intpoi(ant + 12 + poiint(ant + 8), ant);
                intpoi(act,ant + 12 + poiint(ant + 8));
                intpoi(ant, ant + 12 + poiint(ant + 8) + 4);
                intpoi(size, ant + 12 + poiint(ant + 8) + 8);
                return (poiint(ant) + 12);
                ok = 0;

            }
            else if(poiint(act) ==0 && size <= n - act - poiint(act + 8) - 24)
            {
                //verifica daca e momentul si se poate pune la finalul sirului de blocuri
                intpoi(act + poiint(act + 8) + 12, act);
                intpoi(act,act + poiint(act + 8) + 12 + 4);
                intpoi(size,act + poiint(act + 8) + 12 + 8);
                intpoi(0,act + poiint(act + 8) + 12);
                return (poiint(act) + 12);
            }
            if(poiint(act) == 0)ok = 0;
            act = poiint(act);
        }
    }
    return 0;
}

void finalize()
{
    free(arena);
    ind = 0;
}

void allocal(int size, int al)
{
    if(ind == 0 && poiint(8) == 0 && size + 12 <=n)
    {
        //daca se poate pune la inceputul sirului de blocuri
        int b = 12;
        if(12 % al != 0)
            b = ((b / al) + 1) * al;
        if(b + size <= n)
        {
            intpoi(size, b - 4);
            ind = b - 12;
            printf("%d\n", b);
            return;
        }
    }
    else if(size + 12 <= ind)
    {
        // daca se poate aloca in spatele primului bloc
        int b = 12;
        if(b % al != 0)
            b = ((b / al) + 1) * al;
        if(b + size <= ind)
        {
            intpoi(ind, b - 12);
            intpoi(size, b - 4);
            intpoi(b - 12, ind + 4);
            ind = b - 12;
            printf("%d\n", b);
            return;
        }
    }


    int act = ind, ant;
    do
    {
        ant = poiint(act + 4);
        int dim = act - 24 - ant - poiint(ant + 8) - size;
        if(dim  >= 0)
        {
            int b = ant + poiint(ant + 8) + 24;
            if(b % al != 0)
                b = ((b / al) + 1) * al;
            if(b + size <= act)
            {
                //daca se poate insera intre blocuri
                intpoi(act, b - 12);
                intpoi(ant, b - 8);
                intpoi(size, b - 4);
                intpoi(b - 12, ant);
                intpoi(b - 12, act + 4);
                printf("%d\n", b);
                return;
            }
        }
        if(poiint(act) == 0 && size + 12 <= n)
        {
            int b = act + poiint(act + 8) + 24;
            if(b % al != 0)
                b = ((b / al) + 1) * al;
            if(b + size <= n)
            {
                //daca se poate adauga la final
                intpoi(b - 12, act);
                intpoi(act, b - 8);
                intpoi(size, b - 4);
                printf("%d\n", b);
                return;
            }

        }
        act = poiint(act);
    }
    while(act != 0);
    printf("0\n");
}

void realoca(int indx, int size)
{
    int dim = poiint(indx - 4), nouind, i, c = size;
    if(size > dim) c = dim;
    c += indx;
    unsigned char v[size];
    for(i = 0; i < size; i++)v[i] = 0;
    for(i = indx; i < c; i ++)
    {
        v[i - indx] = arena[i];
        arena[i] = 0;
    }

    release(indx);
    nouind = alloc(size);
    if(!nouind)
    {
        printf("0\n");
        return;
    }
    int t = nouind + size;
    for(i = nouind; i < t; i ++)
    {
        arena[i] = v[i - nouind];
    }
    printf("%d\n", nouind);
}

void parse_command(char* cmd)
{
    const char* delims = " \n";

    char* cmd_name = strtok(cmd, delims);
    if (!cmd_name)
    {
        goto invalid_command;
    }

    if (strcmp(cmd_name, "INITIALIZE") == 0)
    {
        char* size_str = strtok(NULL, delims);
        if (!size_str)
        {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        // TODO - INITIALIZE
        initialize(size);

    }
    else if (strcmp(cmd_name, "FINALIZE") == 0)
    {
        // TODO - FINALIZE
        finalize();
    }
    else if (strcmp(cmd_name, "DUMP") == 0)
    {
        // TODO - DUMP
        dump();

    }
    else if (strcmp(cmd_name, "ALLOC") == 0)
    {
        char* size_str = strtok(NULL, delims);
        if (!size_str)
        {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        // TODO - ALLOC
        printf("%d\n", alloc(size));


    }
    else if (strcmp(cmd_name, "FREE") == 0)
    {
        char* index_str = strtok(NULL, delims);
        if (!index_str)
        {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        // TODO - FREE
        release(index);

    }
    else if (strcmp(cmd_name, "FILL") == 0)
    {
        char* index_str = strtok(NULL, delims);
        if (!index_str)
        {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str)
        {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* value_str = strtok(NULL, delims);
        if (!value_str)
        {
            goto invalid_command;
        }
        uint32_t value = atoi(value_str);
        // TODO - FILL
        setval(index, size, value);

    }
    else if (strcmp(cmd_name, "ALLOCALIGNED") == 0)
    {
        char* size_str = strtok(NULL, delims);
        if (!size_str)
        {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* align_str = strtok(NULL, delims);
        if (!align_str)
        {
            goto invalid_command;
        }
        uint32_t align = atoi(align_str);
        // TODO - ALLOCALIGNED
        allocal(size, align);

    }
    else if (strcmp(cmd_name, "REALLOC") == 0)
    {
        //printf("Found cmd REALLOC\n");
        char* index_str = strtok(NULL, delims);
        if (!index_str)
        {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str)
        {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        // TODO - REALLOC
        realoca(index, size);

    }
    else
    {
        goto invalid_command;
    }

    return;

invalid_command:
    printf("Invalid command: %s\n", cmd);
    exit(1);
}

int main(void)
{
    ssize_t read;
    char* line = NULL;
    size_t len;
    /* parse input line by line */
    while ((read = getline(&line, &len, stdin)) != -1)
    {
        // print every command to stdout
        printf("%s", line);
        parse_command(line);
    }

    free(line);
    return 0;
}

