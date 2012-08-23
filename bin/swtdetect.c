#include "ccv.h"
#include <sys/time.h>
#include <ctype.h>
#include <string.h>

typedef struct {
	char *img_fn;
	int img_w;
	int img_h;
	ccv_array_t *words;
	char username[64];
	char text_label[64];
} output_xml_t;

void output_xml(output_xml_t *info)
{
	char xml_fn[64] = {0};
	strcpy(xml_fn, info->img_fn);
	char *cur = strrchr(xml_fn, '.');
	strcpy(cur, ".TextDetct_ccv.xml");
	strcat(cur, "\0");
	FILE* w = fopen(xml_fn, "w");
	fprintf(w, "<?xml version='1.0' encoding='UTF-8'?>\n");
        fprintf(w, "<!--GEDI was developed at Language and Media Processing Laboratory, University of Maryland.-->\n");
        fprintf(w, "<GEDI xmlns='http://lamp.cfar.umd.edu/media/projects/GEDI/' GEDI_version='2.3.24' GEDI_date='08/22/2012'>\n");
	fprintf(w, "	<USER name='%s' date='8/22/2012 15:34' dateFormat='mm/dd/yyyy hh:mm'>	</USER>\n", info->username);
	fprintf(w, "		<DL_DOCUMENT src='%s' docTag='xml' NrOfPages='1'>\n", info->img_fn);
	fprintf(w, "			<DL_PAGE gedi_type='DL_PAGE' src='%s' pageID='1' width='%d' height='%d'>\n", info->img_fn, info->img_w, info->img_h);
	int i;
	for (i = 0; i < info->words->rnum; i++)
	{
		ccv_rect_t* rect = (ccv_rect_t*)ccv_array_get(info->words, i);
		fprintf(w, "			<DL_ZONE gedi_type='TextBox' id='%d' col='%d' row='%d' width='%d' height='%d' Text='%s'> </DL_ZONE>\n", i+1, rect->x, 
rect->y, rect->width, rect->height, info->text_label);
	}
	fprintf(w, "		</DL_PAGE>\n");
	fprintf(w, "	</DL_DOCUMENT>\n");
        fprintf(w, "</GEDI>\n");
	fclose(w);
	printf("XML file '%s' is outputed\n", xml_fn);
}

unsigned int get_current_time()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main(int argc, char** argv)
{
	ccv_swt_param_t params = { .size = 3, .low_thresh = 175, .high_thresh = 320, .max_height = 300, .min_height = 10, .aspect_ratio = 10, .variance_ratio = 0.5, .thickness_ratio = 1.5, .height_ratio = 2, .intensity_thresh = 30, .distance_ratio = 3, .intersect_ratio = 2, .letter_thresh = 3, .elongate_ratio = 1.3, .breakdown = 1, .breakdown_ratio = 12.8 };
	ccv_enable_default_cache();
	ccv_dense_matrix_t* image = 0;
	ccv_read(argv[1], &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
	if (image != 0)
	{
		unsigned int elapsed_time = get_current_time();
		ccv_array_t* words = ccv_swt_detect_words(image, params);
		elapsed_time = get_current_time() - elapsed_time;
		int i;
		for (i = 0; i < words->rnum; i++)
		{
			ccv_rect_t* rect = (ccv_rect_t*)ccv_array_get(words, i);
			printf("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
		}
		printf("total : %d in time %dms\n", words->rnum, elapsed_time);
		// output .xml file
		if (argc == 4)
		{
			output_xml_t info;
			info.img_fn = argv[1];
			info.img_w = image->cols;
			info.img_h = image->rows;
			info.words = words;
			if ((argv[2] == NULL) || (argv[3] == NULL))
			{
				printf("Usages: swtdetect 'filename' 1 'username' 'text_label'\n");
				assert((argv[2] != NULL) && (argv[3] != NULL));
			}
			strcpy(info.username, argv[2]);
			strcpy(info.text_label, argv[3]);
			output_xml(&info);
		}
		ccv_array_free(words);
		ccv_matrix_free(image);
	} else {
		FILE* r = fopen(argv[1], "rt");
		if (argc == 3)
			chdir(argv[2]);
		if(r)
		{
			char file[1000 + 1];
			while(fgets(file, 1000, r))
			{
				int len = (int)strlen(file);
				while(len > 0 && isspace(file[len - 1]))
					len--;
				file[len] = '\0';
				image = 0;
				ccv_read(file, &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
				ccv_array_t* words = ccv_swt_detect_words(image, params);
				int i;
				printf("%s\n%d\n", file, words->rnum);
				for (i = 0; i < words->rnum; i++)
				{
					ccv_rect_t* rect = (ccv_rect_t*)ccv_array_get(words, i);
					printf("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
				}
				ccv_array_free(words);
				ccv_matrix_free(image);
			}
			fclose(r);
		}
	}
	ccv_disable_cache();
	return 0;
}

