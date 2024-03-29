#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libPredictor.h"

typedef struct libPredictor_template_s
{
    char                *name;
    const struct libPredictor_template_s *next;
    const SRT_Predictor_t  predictor;
} libPredictor_template_t;

const libPredictor_template_t mavslmsbank_template
=
{
    .name = "mavslmsbank",
    .next = NULL,
    .predictor = 
    {
        .alloc  =   libPredictor_alloc_MAVSLMSBank,
        .init   =   libPredictor_init_MAVSLMSBank,
        .update =   libPredictor_update_MAVSLMSBank,
        .predict=   libPredictor_predict_MAVSLMSBank,
        .free   =   libPredictor_free_MAVSLMSBank 
        }
};

const libPredictor_template_t mabank2_template
=
{
    .name = "mabank2",
    .next = &mavslmsbank_template,
    .predictor = 
    {
        .alloc  =   libPredictor_alloc_MABank2,
        .init   =   libPredictor_init_MABank2,
        .update =   libPredictor_update_MABank2,
        .predict=   libPredictor_predict_MABank2,
        .free   =   libPredictor_free_MABank2 
    }
};

const libPredictor_template_t mabank_template
=
{
    .name = "mabank",
    .next = &mabank2_template,
    .predictor = 
    {
        .alloc  =   libPredictor_alloc_MABank,
        .init   =   libPredictor_init_MABank,
        .update =   libPredictor_update_MABank,
        .predict=   libPredictor_predict_MABank,
        .free   =   libPredictor_free_MABank 
    }
};

const libPredictor_template_t ma_template
=
{
    .name = "ma",
    .next = &mabank_template,
    .predictor = 
    {
        .alloc  =   libPredictor_alloc_MA,
        .init   =   libPredictor_init_MA,
        .update =   libPredictor_update_MA,
        .predict=   libPredictor_predict_MA,
        .free   =   libPredictor_free_MA
        }
};

const libPredictor_template_t template_template
=
{
    .name = "template",
    .next = &ma_template,
    .predictor = 
    {
        .alloc  =   libPredictor_alloc_template,
        .init   =   libPredictor_init_template,
        .update =   libPredictor_update_template,
        .predict=   libPredictor_predict_template,
        .free   =   libPredictor_free_template 
        }
};

const libPredictor_template_t head
=
{
    .name = "Head",
    .next = &template_template,
    .predictor = {0}
};

int libPredictor_getPredictor(SRT_Predictor_t *ppredictor, char *name)
{
    int ret;
    
    int found = 0;
    const libPredictor_template_t *current = &head;

    *ppredictor = (SRT_Predictor_t){0};
    
    while(NULL != current->next)
    {
        current = current->next;
        
        ret = strcmp(current->name, name);
        if(0 == ret)
        {
            /*Copying the entire struct*/
            *ppredictor = current->predictor;
            found = 1;
            break;
        }
    }
    
    if(0 != found)
    {
        ppredictor->state = ppredictor->alloc();
        if(NULL == ppredictor->state)
        {
            fprintf(stderr, "libPredictor_getPredictor: The \"alloc\" function failed for "
                            "\"%s\".\n", name);
            ret = 1;
            goto exit0;
        }
        
        ppredictor->init(ppredictor->state);
    }
    else
    {
        fprintf(stderr, "libPredictor_getPredictor: No predictor named \"%s\" was found.\n",
                        name);
        ret = -1;
        goto exit0;
    }
    
exit0:
    return ret;
}

