#include "sicm_low.h"
#include "dram.h"
#include "knl_hbm.h"

#include <numaif.h>

void* sicm_alloc(struct sicm_device* device, size_t size) {
  return device->alloc(device, size);
}

void sicm_free(struct sicm_device* device, void* ptr, size_t size) {
  device->free(device, ptr, size);
}

size_t sicm_used(struct sicm_device* device) {
  return device->used(device);
}

size_t sicm_capacity(struct sicm_device* device) {
  return device->capacity(device);
}

int sicm_add_to_bitmask(struct sicm_device* device, struct bitmask* mask) {
  return device->add_to_bitmask(device, mask);
}

int sicm_move(struct sicm_device* src, struct sicm_device* dest, void* ptr, size_t len) {
  if(src->move_ty == SICM_MOVER_NUMA && dest->move_ty == SICM_MOVER_NUMA) {
    int dest_node = dest->move_payload.numa;
    int nodemask_length = numa_max_node() / (sizeof(long int) * 8) + 1;
    unsigned long* nodes = malloc(nodemask_length);
    int i = nodemask_length - 1;
    while(dest_node > 0) {
      if(dest_node > sizeof(long int) * 8) {
        i--;
        dest_node -= sizeof(long int) * 8;
      }
      else {
        nodes[i] = 1 << dest_node;
        dest_node = -1;
      }
    }
    int res = mbind(ptr, len, MPOL_BIND, nodes, numa_max_node() + 1, MPOL_MF_MOVE);
    free(nodes);
    return res;
  }
  return -1;
}

int zero() {
  return 0;
}

int sicm_cpu_mask_created = 0;
struct bitmask* sicm_cpu_mask_memo;

struct bitmask* sicm_cpu_mask() {
  if(sicm_cpu_mask_created) return sicm_cpu_mask_memo;
  
  struct bitmask* cpumask = numa_allocate_cpumask();
  int cpu_count = numa_num_possible_cpus();
  int node_count = numa_max_node() + 1;
  sicm_cpu_mask_memo = numa_bitmask_alloc(node_count);
  int i, j;
  for(i = 0; i < node_count; i++) {
    numa_node_to_cpus(i, cpumask);
    for(j = 0; j < cpu_count; j++) {
      if(numa_bitmask_isbitset(cpumask, j)) {
        numa_bitmask_setbit(sicm_cpu_mask_memo, i);
        break;
      }
    }
  }
  numa_free_cpumask(cpumask);
  return sicm_cpu_mask_memo;
}

int main() {
  int spec_count = 2;
  struct sicm_device_spec specs[] = {sicm_knl_hbm_spec(), sicm_dram_spec()};
  
  int i;
  int non_numa = 0;
  for(i = 0; i < spec_count; i++)
    non_numa += specs[i].non_numa_count();
  
  int device_count = non_numa + numa_max_node() + 1;
  struct sicm_device* devices = malloc(device_count * sizeof(struct sicm_device));
  struct bitmask* numa_mask = numa_bitmask_alloc(numa_max_node() + 1);
  int idx = 0;
  for(i = 0; i < spec_count; i++)
    idx = specs[i].add_devices(devices, idx, numa_mask);
  
  /*
   * test code starts here
   * everything above this comment is required spinup
   */
  printf("used: %lu\n", sicm_used(&devices[0]));
  printf("capacity: %lu\n", sicm_capacity(&devices[0]));
  int* test = sicm_alloc(&devices[0], 100 * sizeof(int));
  for(i = 0; i < 100; i++)
    test[i] = i;
  for(i = 0; i < 100; i++)
    printf("%d ", test[i]);
  printf("\n");
  sicm_free(&devices[0], test, 100 * sizeof(int));
  return 1;
}
