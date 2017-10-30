#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <mpi.h>

#include <list>

using namespace std;

#if 0
#include <fcntl.h>
#include <sys/time.h>


#include <map>
#include <set>

//#define DEBUG_READER

map<int64_t,int64_t> rids[4];
#endif /* 0 */

char *me;
int myrank, worldsz;
list<char*> files;

void usage(int ret)
{
    printf("\n"
           "usage: %s [options] -i input_dir -o out_dir\n"
           "  options:\n"
           "    -h        This usage info\n"
           "\n",
           me);

    exit(ret);
}

int listdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return 1;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[PATH_MAX];
            if (strncmp(entry->d_name, ".", 2) == 0 ||
                strncmp(entry->d_name, "..", 3) == 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            //printf("%*s[%s]\n", indent, "", entry->d_name);
            if (listdir(path, indent + 2))
                return 1;
        } else {
            /* It's a file; store its path */
            if (strncmp(entry->d_name + strlen(entry->d_name) - 3, "crc", 3) == 0)
                continue;

            char *path = (char *)malloc(PATH_MAX);
            if (!path)
                return 1;

            snprintf(path, PATH_MAX, "%s/%s", name, entry->d_name);
            files.push_back(path);
            //printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char **argv)
{
    int ret, c;
    char indir[PATH_MAX], outdir[PATH_MAX];
    list<char*>::iterator it;

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Init failed\n");
        return 1;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &worldsz);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    me = argv[0];
    indir[0] = outdir[0] = '\0';

    while ((c = getopt(argc, argv, "hi:o:")) != -1) {
        switch(c) {
        case 'h': /* print help */
            usage(0);
        case 'i': /* input directory (VPIC output) */
            if (!strncpy(indir, optarg, PATH_MAX)) {
                perror("Error: invalid input dir");
                usage(1);
            }
            break;
        case 'o': /* output directory (trajectory files) */
            if (!strncpy(outdir, optarg, PATH_MAX)) {
                perror("Error: invalid output dir");
                usage(1);
            }
            break;
        default:
            usage(1);
        }
    }

    if (!indir[0]) {
        fprintf(stderr, "Error: input directory unspecified\n");
        usage(1);
    }

    if (myrank == 0 && listdir(indir, 0)) {
        /* Clean up list */
        fprintf(stderr, "Error, clean up list!\n");
        ret = 1;
    }

    if (myrank == 0 && !ret) {
        for (it = files.begin(); it != files.end(); ++it) {
            printf("  %s\n", *it);
        }
    }

    MPI_Finalize();
    return ret;
}
