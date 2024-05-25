#include <assert.h>
#include <dpu.h>
#include <dpu_log.h>
#include <stdio.h>
#include <unistd.h>

#include "common/params.h"
#include "common/hilbert.h"
#include "common/rtree.h"
#include "dpu_bin/rtree_load/hilbert.h"
#include "dpu_bin/rtree_load/zorder.h"
#include "dpu_bin/rtree_query.h"

int main(int argc, char *argv[]) {
	struct dpu_set_t set, dpu;

// == Data Parse == //
	FILE* csv_file = fopen("uniform.csv", "rb");

	fseek(csv_file, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(csv_file, 0, SEEK_SET)

	char *raw_data = malloc(fsize+1);
	fread(raw_data, fsize, 1, csv_file);
	fclose(csv_file);
	raw_data[fsize] = 0;

	DPU_ASSERT(dpu_alloc(MAX_NR_DPUS, NULL, &set));
	DPU_ASSERT(dpu_load_from_incbin(set, parse_csv, NULL));
	
	size_t dpu_idx;
	size_t buffer_len[MAX_NR_DPUS];
	size_t chars_per_dpu = fsize/MAX_NR_DPUS +1;
	DPU_FOREACH(set, dpu, dpu_idx) {
		size_t buffer_start = chars_per_dpu * dpu_idx;
		buffer_len[dpu_idx] = chars_per_dpu;
		if (dpu_idx) {
            for (; raw_data[buffer_start] != '\n'; buffer_start++) buffer_len[dpu_idx]--;
        }
        if (buffer_start + chars_per_dpu < fsize) {
            for (
				; 
				raw_data[buffer_start+buffer_len[dpu_idx]-1] != '\n';
				buffer_len[dpu_idx]++;
			);
        }
		dpu_prepare_xfer(dpu, &raw_data[buffer_start]);
	}
	//32 is (hopefully) the longest line length
	DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "buffer", 0, chars_per_dpu+32, DPU_XFER_DEFAULT));

	DPU_FOREACH(set,dpu,dpu_idx) {
		dpu_prepare_xfer(dpu, &buffer_len[dpu_idx]);
	}
	DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "buffer_size", 0, sizeof(size_t), DPU_XFER_DEFAULT));
	DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));

	free(raw_data);

	size_t nr_pts = 0;
	size_t dpu_pt_ct[MAX_NR_DPUS];
	DPU_FOREACH(set,dpu,dpu_idx) {
		dpu_copy_from(dpu,"pt_pos",0,&dpu_pt_ct[dpu_idx],sizeof(size_t));
		nr_pts = dpu_pt_ct[dpu_idx] + 1;
	}
	indexedpt_t * pts = malloc(sizeof(indexedpt_t) * nr_pts);
	DPU_FOREACH(set,dpu,dpu_idx) {
		dpu_copy_from(dpu,"parsed_pts",0,pts[dpu_pt_ct[dpu_idx]])
	}
	
// == Z-Order Map == //
	// indexedpt_t * points[NUM_PTS];
	// char line[1024];
	// for (size_t i = 0; i<NUM_PTS; i+=2)
	// {
	// 	fgets(line, 1024, csv_file);
	// 	//TODO this assumes a lot about the data set. fix that. assumptions:
	// 	//		1. all data pts are the same no. of digits, and
	// 	//		2. no data pts are negative
	// 	char * c = line+2;
	// 	for (size_t j = 0; *c != '.' && j < 1024; j++) c++; //march ptr to second dec pt
	// 	points[i].y = atoll(c+1);
	// 	points[i].x = atoll(line+2);
	// }

	size_t pts_per_dpu = nr_pts / MAX_NR_DPUS + 1;
	size_t dpu_idx;
	DPU_FOREACH(set, dpu, dpu_idx) {
		dpu_prepare_xfer(dpu, &pts[pts_per_dpu*dpu_idx]);
		if (dpu_idx == MAX_NR_DPUS - 1) {
			dpu_
		}
	}
	DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "buffer", 0, BUFFER_SIZE * sizeof(indexedpt_t), DPU_XFER_DEFAULT));

	DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
	
	DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, "buffer", 0, BUFFER_SIZE * sizeof(indexedpt_t), DPU_XFER_DEFAULT))

	DPU_ASSERT(dpu_free(set));

	return 0;
}