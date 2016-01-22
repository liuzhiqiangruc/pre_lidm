/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lda.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-03-26
 *   info     :
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "str.h"
#include "lda.h"

#define LDA_LINE_LEN 1024
static void malloc_space(Lda *lda) {
    lda->id_doc_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->d);
    memset(lda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * lda->d);

    lda->id_v_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->v);
    memset(lda->id_v_map, 0, sizeof(char[KEY_SIZE]) * lda->v);

    lda->tokens = (int (*)[3]) malloc(sizeof(int[3]) * lda->t);
    memset(lda->tokens, 0, sizeof(int[3]) * lda->t);

    lda->nd = (ModelEle *) malloc(sizeof(ModelEle) * lda->d * (lda->p.k + 1));
    memset(lda->nd, 0, sizeof(ModelEle) * lda->d * (lda->p.k + 1));

    lda->nw = (ModelEle *) malloc(sizeof(ModelEle) * lda->v * (lda->p.k + 1));
    memset(lda->nw, 0, sizeof(ModelEle) * lda->v * (lda->p.k + 1));

    lda->nkw = (int *) malloc(sizeof(int) * (lda->p.k + 1));
    memset(lda->nkw, 0, sizeof(int) * (lda->p.k + 1));
}

static void fullfill_param(Lda *lda) {
    for (int i = 0; i < lda->t; i++) {
        int uid = lda->tokens[i][0];
        int vid = lda->tokens[i][1];
        int top = lda->tokens[i][2];
        lda->nd[uid * (lda->p.k + 1) + top].count += 1;
        lda->nw[vid * (lda->p.k + 1) + top].count += 1;
        lda->nkw[top] += 1;
    }
    for (int d = 0; d < lda->d; d++){
        int offs = d * (lda->p.k + 1);
        int p = 0;
        for (int k = 1; k <= lda->p.k; k++){
            if (lda->nd[offs + k].count > 0){
                lda->nd[offs + p].next = k;
                lda->nd[offs + k].prev = p;
                p = k;
            }
        }
        lda->nd[offs + p].next = 0;
        lda->nd[offs].prev = p;
    }
    for (int v = 0; v < lda->v; v++){
        int offs = v * (lda->p.k + 1);
        int p = 0;
        for (int k = 1; k <= lda->p.k; k++){
            if (lda->nw[offs + k].count > 0){
                lda->nw[offs + p].next = k;
                lda->nw[offs + k].prev = p;
                p = k;
            }
        }
        lda->nw[offs + p].next = 0;
        lda->nw[offs].prev = p;
    }
    save_lda(lda, 0);
}

static void shuffle(int *a, int n) {
    int *tmp = a;
    int i, t, k;
    i = t = 0;
    for (k = n; k >= 2; k--) {
        i = rand() % k;
        t = tmp[i];
        tmp[i] = tmp[0];
        *tmp++ = t;
    }
}

Lda *create_lda() {
    Lda *lda = (Lda *) malloc(sizeof(Lda));
    memset(lda, 0, sizeof(Lda));
    return lda;
}

int init_lda(Lda *lda) {
    FILE *fp = NULL;
    if (NULL == (fp = fopen(lda->p.in_file, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", lda->p.in_file);
        return -1;
    }
    char buffer[LDA_LINE_LEN] = {'\0'};
    char * string = NULL, *token = NULL;
    int token_size = 0;
    Hash * uhs = hash_create(1<<20, STRING);
    Hash * vhs = hash_create(1<<20, STRING);
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        hash_add(uhs, token);
        token = strsep(&string, "\t");
        hash_add(vhs, token);
        token_size += 1;
    }
    lda->d = hash_cnt(uhs);
    lda->v = hash_cnt(vhs);
    lda->t = token_size;
    malloc_space(lda);

    rewind(fp);
    int uid = -1, vid = -1, tid = -1;
    int token_index = 0;
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        uid = hash_find(uhs, token);
        if (lda->id_doc_map[uid][0] == '\0'){
            strncpy(lda->id_doc_map[uid], token, KEY_SIZE - 1);
        }
        lda->tokens[token_index][0] = uid;
        token = strsep(&string, "\t");
        vid = hash_find(vhs, token);
        if (lda->id_v_map[vid][0] == '\0'){
            strncpy(lda->id_v_map[vid], token, KEY_SIZE - 1);
        }
        lda->tokens[token_index][1] = vid;
        tid = (int) ((1.0 + rand()) / (1.0 + RAND_MAX) * (lda->p.k));
        tid += 1;
        token = strsep(&string, "\t");
        if (token){
            tid = atoi(token);
        }
        lda->tokens[token_index][2] = tid;
        token_index += 1;
    }
    fclose(fp);
    hash_free(uhs);    uhs = NULL;
    hash_free(vhs);    vhs = NULL;
    return 0;
}
void est_lda(Lda *lda) {

    fullfill_param(lda);

    // todo

    /*
    int *p = (int *) malloc(sizeof(int) * lda->t);
    int st = 0;

    double *prob = (double *) malloc(sizeof(double) * lda->p.k);
    double vb = lda->p.b * lda->v;

    for (int i = 0; i < lda->t; i++) p[i] = i;
    for (int i = 1; i <= lda->p.niters; i++) {
        fprintf(stderr, "iteration %d estimate begin ... ", i);
        shuffle(p, lda->t);
        for (int j = 0; j < lda->t; j++) {
            int id = p[j];
            int uid = lda->tokens[id][0];
            int vid = lda->tokens[id][1];
            int top = lda->tokens[id][2];

            lda->nd[uid * lda->p.k + top] -= 1;
            lda->nw[vid * lda->p.k + top] -= 1;
            lda->nkw[top] -= 1;

            for (int l = 0; l < lda->p.k; l++) {
                prob[l] = 1.0 * (lda->nd[uid * lda->p.k + l] + lda->p.a) *
                                (lda->nw[vid * lda->p.k + l] + lda->p.b) /
                                (lda->nkw[l] + vb);
                if (l > 0) prob[l] += prob[l - 1];
            }
            double rnd = prob[lda->p.k - 1] * (0.1 + rand()) / (0.1 + RAND_MAX);
            for (st = 0; st < lda->p.k; st++) {
                if (prob[st] > rnd) break;
            }

            lda->nd[uid * lda->p.k + st] += 1;
            lda->nw[vid * lda->p.k + st] += 1;
            lda->nkw[st] += 1;

            lda->tokens[id][2] = st;
        }

        fprintf(stderr, " done\n");
        if (i % lda->p.savestep == 0) {
            save_lda(lda, i);
        }
    }
    free(p);    p    = NULL;
    free(prob); prob = NULL;
    */
}

void save_lda(Lda *lda, int n) {
    FILE *fp = NULL;
    char nd_file[512];
    char nw_file[512];
    char tk_file[512];

    if (n < lda->p.niters){
        sprintf(nd_file,  "%s/%d_doc_topic",  lda->p.out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", lda->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",lda->p.out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",  lda->p.out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", lda->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",lda->p.out_dir, "f");
    }

    //output for nd
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nd_file);
        return;
    }
    for (int d = 0; d < lda->d; d++) {
        fprintf(fp, "%s", lda->id_doc_map[d]);
        int offs = d * (lda->p.k + 1);
        for (int k = 1; k <= lda->p.k; k++) {
            fprintf(fp, "\t%d", lda->nd[offs + k].count);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for nw
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw_file);
        return;
    }
    for (int v = 0; v < lda->v; v++) {
        fprintf(fp, "%s", lda->id_v_map[v]);
        int offs = v * (lda->p.k + 1);
        for (int k = 1; k <= lda->p.k; k++) {
            fprintf(fp, "\t%d", lda->nw[offs + k].count);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for tk
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (int t = 0; t < lda->t; t++) {
        fprintf(fp, "%s\t%s\t%d\n",   lda->id_doc_map[lda->tokens[t][0]],  \
                                      lda->id_v_map[lda->tokens[t][1]],    \
                                      lda->tokens[t][2]);
    }
    fclose(fp);
}

void free_lda(Lda *lda) {
    if (lda) {
        if (lda->id_doc_map) {
            free(lda->id_doc_map);
            lda->id_doc_map = NULL;
        }
        if (lda->id_v_map) {
            free(lda->id_v_map);
            lda->id_v_map = NULL;
        }
        if (lda->tokens){
            free(lda->tokens);
            lda->tokens = NULL;
        }
        if (lda->nd) {
            free(lda->nd);
            lda->nd = NULL;
        }
        if (lda->nw) {
            free(lda->nw);
            lda->nw = NULL;
        }
        if (lda->nkw) {
            free(lda->nkw);
            lda->nkw = NULL;
        }
        free(lda);
    }
}
