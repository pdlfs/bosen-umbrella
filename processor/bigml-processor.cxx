#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <mpi.h>
#include <list>
using namespace std;

#define MD5FNAME    "md5.sigs"
//#define DEBUG_PROCESSOR

char *me;
int dryrun;
int myrank;
int worldsz;
list<char*> files;

/*
 * usage: print usage blob
 */
void usage(void)
{
    printf("\n"
           "usage: %s [options] -i input_dir -o out_dir\n"
           "  options:\n"
           "    -d        Dry run: no actions applied on data\n"
           "    -h        This usage info\n"
           "\n",
           me);

    abort();
}

/*
 * msg_abort: abort with a message
 */
static inline void msg_abort(const char* msg)
{
    if (errno != 0) {
        fprintf(stderr, "Error: %s (%s)\n", msg, strerror(errno));   
    } else {
        fprintf(stderr, "Error: %s\n", msg);
    }

    abort();
}

int listdir(const char *name, int *idx)
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
#ifdef DEBUG_PROCESSOR
            printf("[%s]\n", path);
#endif
            if (listdir(path, idx))
                return 1;
        } else {
            /* It's a file; store its path */
            if (strncmp(entry->d_name+strlen(entry->d_name)-3, "crc", 3) == 0)
                continue;

            /*
             * Only store N/P paths, where N = number of paths,
             *                             P = number of MPI processes
             */
            if (!*idx) {
                char *path = (char *)malloc(PATH_MAX);
                if (!path)
                    return 1;

                snprintf(path, PATH_MAX, "%s/%s", name, entry->d_name);
                files.push_back(path);
#ifdef DEBUG_PROCESSOR
                printf(" %s\n", path);
#endif
                *idx = worldsz;
            }
            (*idx)--;
        }
    }
    closedir(dir);
    return 0;
}

int process_file(char *path, char *indir, char *outdir)
{
    char *rpath;
    char srcp[PATH_MAX], dstp[PATH_MAX];

    /* Extract relative path to indir */
    rpath = strstr(path, indir) + strlen(indir) + 1;
#ifdef DEBUG_PROCESSOR
    printf("%d: Relative path %s\n", myrank, rpath);
#endif
    snprintf(srcp, sizeof(srcp), "%s/%s", indir, rpath);
    snprintf(dstp, sizeof(dstp), "%s/%s.gz", outdir, rpath);

    /* If it's a dry run, then just print what we would do */
    if (dryrun) {
        printf("File: %s\n   ~> %s\n", srcp, dstp);
        return 0;
    }

    /*
     * We handle each file the same way:
     * 1) Compress and replace old file
     * 2) Compute MD5 hash
     * 3) Append MD5 hash to MD5 hash file
     */
    char syscmd[3*PATH_MAX];

    snprintf(syscmd, sizeof(syscmd), "mkdir -p $(dirname %s)", dstp);
#ifdef DEBUG_PROCESSOR
    fprintf(stderr, "Executing: %s\n", syscmd);
#endif
    if (system(syscmd))
        msg_abort("mkdir failed");

    snprintf(syscmd, sizeof(syscmd), "gzip < %s > %s", srcp, dstp);
#ifdef DEBUG_PROCESSOR
    fprintf(stderr, "Executing: %s\n", syscmd);
#endif
    if (system(syscmd))
        msg_abort("gzip failed");

    snprintf(syscmd, sizeof(syscmd), "md5sum %s | sed 's/  /:/g' >> %s/"
                                     MD5FNAME ".%05d", dstp, outdir, myrank);
#ifdef DEBUG_PROCESSOR
    fprintf(stderr, "Executing: %s\n", syscmd);
#endif
    if (system(syscmd))
        msg_abort("md5sum failed");

    return 0;
}

void create_local_md5file(char *outdir)
{
    char syscmd[2*PATH_MAX];

    snprintf(syscmd, sizeof(syscmd), "> %s/" MD5FNAME ".%05d", outdir, myrank);
#ifdef DEBUG_PROCESSOR
    fprintf(stderr, "Executing: %s\n", syscmd);
#endif
    if (system(syscmd))
        msg_abort("create_local_md5file: file creation failed");
}

void merge_md5files(char *outdir)
{
    char syscmd[3*PATH_MAX];

    snprintf(syscmd, sizeof(syscmd), "> %s/" MD5FNAME, outdir);
#ifdef DEBUG_PROCESSOR
    fprintf(stderr, "Executing: %s\n", syscmd);
#endif
    if (system(syscmd))
        msg_abort("merge_md5files: file creation failed");

    for (int i = 0; i < worldsz; i++) {
        snprintf(syscmd, sizeof(syscmd), "cat %s/" MD5FNAME ".%05d >> %s/"
                                         MD5FNAME, outdir, i, outdir);
#ifdef DEBUG_PROCESSOR
        fprintf(stderr, "Executing: %s\n", syscmd);
#endif
        if (system(syscmd))
            msg_abort("merge_md5files: file append failed");

        snprintf(syscmd, sizeof(syscmd), "rm %s/" MD5FNAME ".%05d", outdir, i);
#ifdef DEBUG_PROCESSOR
        fprintf(stderr, "Executing: %s\n", syscmd);
#endif
        if (system(syscmd))
            msg_abort("merge_md5files: file removal failed");
    }
}

int main(int argc, char **argv)
{
    int ret, c, idx;
    char indir[PATH_MAX], outdir[PATH_MAX];

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Init failed\n");
        return 1;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &worldsz);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    /* Check that we have a shell available to us */
    if (!system(NULL))
        msg_abort("system unavailable");

#ifdef DEBUG_PROCESSOR
    printf("Info: worldsz = %d, myrank = %d\n", worldsz, myrank);
#endif

    me = argv[0];
    dryrun = 0;
    indir[0] = outdir[0] = '\0';

    while ((c = getopt(argc, argv, "hdi:o:")) != -1) {
        switch(c) {
        case 'h': /* print help */
            usage();
        case 'd': /* this is a dry run */
            dryrun = 1;
            break;
        case 'i': /* input directory */
            if (!strncpy(indir, optarg, PATH_MAX)) {
                perror("Error: invalid input dir");
                usage();
            }
            break;
        case 'o': /* output directory */
            if (!strncpy(outdir, optarg, PATH_MAX)) {
                perror("Error: invalid output dir");
                usage();
            }
            break;
        default:
            usage();
        }
    }

    if (!indir[0] || !outdir[0]) {
        fprintf(stderr, "Error: input directory unspecified\n");
        usage();
    }

    if (!dryrun)
        create_local_md5file(outdir);

    list<char*>::iterator it;
    int fcount = 0;

    idx = myrank;
    ret = listdir(indir, &idx);
    if (ret)
        goto cleanup;

    for (it = files.begin(); it != files.end(); ++it) {
        if (process_file(*it, indir, outdir)) {
            fprintf(stderr, "%d: process_file failed\n", myrank);
            break;
        }

        ++fcount;
        printf("%d: %d of %d (%3.2f%%)\n", myrank, fcount,
               (int) files.size(), fcount * 100.0 / files.size());
    }

cleanup:
    while (!files.empty()) {
#ifdef DEBUG_PROCESSOR
        printf("%d: %s\n", myrank, files.front());
#endif
        free(files.front());
        files.pop_front();
    }

    /* Root process collects all MD5 signatures in one file */
    if (!dryrun && !myrank)
        merge_md5files(outdir);

    MPI_Finalize();
    return ret;
}
