#include <tuple>
#include <iostream>
#include <string.h>
#include <set>
using namespace std;

/*
tuple<double, double, double> boundary_point;
*/
//set<tuple<double, double, double> > boundary_points;
set<long> boundary_index;
/*
void setBoundaryPoints(char* input){
    FILE* infile = fopen(input, "r+");
    char buffer[50];
    int i = 0;
    double a[3];
    while (fscanf(infile, "%s", buffer) != EOF){
        a[i] = atoi(buffer);
        i++;
        if (i == 3){
            i = 0;
            boundary_points.insert(make_tuple(a[0], a[1], a[2]));
        }
    }    
} 
*/

int finsert (FILE* file, const char *buffer) {

    long int insert_pos = ftell(file);
    if (insert_pos < 0) return insert_pos;

    // Grow from the bottom
    int seek_ret = fseek(file, 0, SEEK_END);
    if (seek_ret) return seek_ret;
    long int total_left_to_move = ftell(file);
    if (total_left_to_move < 0) return total_left_to_move;

    char move_buffer[1024];
    long int ammount_to_grow = strlen(buffer);
    if (ammount_to_grow >= sizeof(move_buffer)) return -1;

    total_left_to_move -= insert_pos;

    for(;;) {
        long ammount_to_move = sizeof(move_buffer);
        if (total_left_to_move < ammount_to_move) ammount_to_move = total_left_to_move;

        long int read_pos = insert_pos + total_left_to_move - ammount_to_move;

        seek_ret = fseek(file, read_pos, SEEK_SET);
        if (seek_ret) return seek_ret;
        fread(move_buffer, ammount_to_move, 1, file);
        if (ferror(file)) return ferror(file);

        seek_ret = fseek(file, read_pos + ammount_to_grow, SEEK_SET);
        if (seek_ret) return seek_ret;
        fwrite(move_buffer, ammount_to_move, 1, file);
        if (ferror(file)) return ferror(file);

        total_left_to_move -= ammount_to_move;

        if (!total_left_to_move) break;

    }
    seek_ret = fseek(file, insert_pos, SEEK_SET);
    if (seek_ret) return seek_ret;
    fwrite(buffer, ammount_to_grow, 1, file);
    if (ferror(file)) return ferror(file);

    return 0;
}


void setBoundaryIndex(char* input){
    FILE* infile = fopen(input, "r+");
    char buffer[50];
    while (fscanf(infile, "%s", buffer) != EOF){
        boundary_index.insert(atoi(buffer)); 
    }
}

void wash(char* input, char* output){
    size_t deleted = 0;
    size_t vertex_num, face_num, edge_num;
    FILE* infile = fopen(input, "r+");
    FILE* outfile = fopen(output, "w+");
    char buffer[50];
    fscanf(infile, "%s", buffer);
    fscanf(infile, "%s", buffer);
    vertex_num = atoi(buffer);
    fscanf(infile, "%s", buffer);
    face_num = atoi(buffer);
    fscanf(infile, "%s", buffer);
    edge_num = atoi(buffer);

    size_t v_times = 3 * vertex_num;
    int cnt = 0;
    for (size_t i = 0; i < v_times; i++){
        fscanf(infile, "%s", buffer);
        fprintf(outfile, "%s ", buffer);
        cnt++;
        if (cnt == 3){
            cnt = 0;
            fprintf(outfile, "\n");
        }
    }
    
    for (size_t i = 0; i < face_num; i++){
        char buffer1[50];
        char buffer2[50];
        char buffer3[50];
        char buffer4[50];
        fscanf(infile, "%s", buffer1);
        fscanf(infile, "%s", buffer2);
        fscanf(infile, "%s", buffer3);
        fscanf(infile, "%s", buffer4);
        long a = atoi(buffer2);
        long b = atoi(buffer3);
        long c = atoi(buffer4);
        if (boundary_index.find(a) != boundary_index.end() 
                && boundary_index.find(b) != boundary_index.end()
                && boundary_index.find(c) != boundary_index.end()){
            deleted++;
        }
        else{
            fprintf(outfile, "%s ", buffer1);
            fprintf(outfile, "%s ", buffer2);
            fprintf(outfile, "%s ", buffer3);
            fprintf(outfile, "%s ", buffer4);
            fprintf(outfile, "\n");
        }
    }
    face_num -= deleted;

    fseek(outfile, 0, SEEK_SET);
    char str[100];
    snprintf(str, sizeof(str), "%zu %zu %zu\n", vertex_num, face_num, edge_num);
    finsert(outfile, str);
    fseek(outfile, 0, SEEK_SET);
    finsert(outfile, "OFF\n");

}

int main(int argc, char** argv){
    char* boudary = argv[1];
    char* input = argv[2];
    char* output = argv[3];

    setBoundaryIndex(boudary);
    wash(input, output);
}
