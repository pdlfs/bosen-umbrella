#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <mpi.h>
#include <list>
using namespace std;

#define MD5FNAME    "md5.sigs"
//#define DEBUG_VERIFIER

char *me;
int dryrun;
int xtract;
int myrank;
int worldsz;
FILE *md5fp;
list<char*> files;

/*
 * usage: print usage blob
 */
void usage(void)
{
    printf("\n"
           "usage: %s [options] -i input_dir\n"
           "  options:\n"
           "    -d        Dry run: no actions applied on data\n"
           "    -x        Decompress all data\n"
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

int pick_files(void)
{
    int idx = myrank;
    char buf[PATH_MAX+256];

    while(fscanf(md5fp, "%s", buf) != EOF) {
        buf[32] = '\0';

        /*
         * Only process N/P paths, where N = number of paths,
         *                               P = number of MPI processes
         */
        if (!idx) {
            char *path = (char *)malloc(PATH_MAX+256);
            if (!path)
                return 1;

            memcpy(path, buf, sizeof(buf));
            files.push_back(path);
            idx = worldsz;
        }
        idx--;
    }

    return 0;
}

int process_file(char *buf, char *indir)
{
    char *hash, *fpath;
    char syscmd[2*PATH_MAX];
    int md5ret = 0;

    hash = buf;
    fpath = buf+33;

#ifdef DEBUG_VERIFIER
    fprintf(stdout, "File = %s\n", fpath);
    fprintf(stdout, "\tMD5 hash: %s\n\n", hash);
#endif

    /* If it's a dry run, then just print what we would do */
    if (dryrun) {
        printf("File: %s\n", fpath);
        return 0;
    }

    /* Check md5 hash */
    snprintf(syscmd, sizeof(syscmd), "echo \"%s %s/%s\" | md5sum -c > /dev/null",
             hash, indir, fpath);
#ifdef DEBUG_VERIFIER
    fprintf(stderr, "Executing: %s\n", syscmd);
#endif
    md5ret = system(syscmd);
    if (md5ret) {
        fprintf(stderr, "Error: md5sum failed for %s/%s\n", indir, fpath);
    } else if (xtract) {
        /* MD5 sum matched; extract data in place */
        snprintf(syscmd, sizeof(syscmd), "gunzip -k %s/%s", indir, fpath);
#ifdef DEBUG_VERIFIER
        fprintf(stderr, "Executing: %s\n", syscmd);
#endif
        if (system(syscmd))
            fprintf(stderr, "Error: failed to extract data of %s/%s\n",
                    indir, fpath);
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret, c;
    char indir[PATH_MAX], temp[PATH_MAX];
    char syscmd[PATH_MAX+256];

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Init failed\n");
        return 1;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &worldsz);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    /* Check that we have a shell available to us */
    if (!system(NULL))
        msg_abort("system unavailable");

#ifdef DEBUG_VERIFIER
    printf("Info: worldsz = %d, myrank = %d\n", worldsz, myrank);
#endif

    me = argv[0];
    dryrun = xtract = 0;
    indir[0] = '\0';

    while ((c = getopt(argc, argv, "hdxi:")) != -1) {
        switch(c) {
        case 'h': /* print help */
            usage();
        case 'd': /* this is a dry run */
            dryrun = 1;
            break;
        case 'x': /* extract data */
            xtract = 1;
            break;
        case 'i': /* input directory */
            if (!strncpy(indir, optarg, PATH_MAX)) {
                perror("Error: invalid input dir");
                usage();
            }
            break;
        default:
            usage();
        }
    }

    if (!indir[0]) {
        fprintf(stderr, "Error: input directory unspecified\n");
        usage();
    }

    snprintf(temp, sizeof(temp), "%s/" MD5FNAME, indir);
    if (!(md5fp = fopen(temp, "r")))
        msg_abort("fopen failed");

    list<char*>::iterator it;
    int fcount = 0;

    ret = pick_files();
    if (ret)
        goto cleanup;

    for (it = files.begin(); it != files.end(); ++it) {
        if (process_file(*it, indir)) {
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

    fclose(md5fp);
    MPI_Finalize();
    return ret;
}
