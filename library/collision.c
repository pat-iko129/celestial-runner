#include "collision.h"
#include "list.h"
#include "math.h"
#include "vector.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * Find the orthogonal vector then normalize it
 */
vector_t get_ortho_vec(vector_t p1, vector_t p2) {
  vector_t ortho = vec_subtract(p1, p2);
  ortho = orthonormal_vec(ortho);

  return ortho;
}

// dot product each vectex onto orthonormal vector
// keep updating min and max values until done with one shape
// save vector between min and max
vector_t find_projection(list_t *shape, vector_t orthogonal) {
  double min = INFINITY;
  double max = -INFINITY;

  for (size_t i = 0; i < list_size(shape); i++) {
    double projection = vec_dot(*(vector_t *)list_get(shape, i), orthogonal);
    if (projection < min) {
      min = projection;
    }
    if (projection > max) {
      max = projection;
    }
  }
  return (vector_t){min, max};
}

collision_info_t check_shape(list_t *shape1, list_t *shape2) {
  size_t len1 = list_size(shape1);
  collision_info_t collision;
  vector_t min_projection;
  double min_projection_amount = INFINITY;

  // for loop per normal vector
  for (size_t i = 0; i < len1; i++) {
    vector_t p1 = *(vector_t *)list_get(shape1, i);
    vector_t p2 = *(vector_t *)list_get(shape1, (i + 1) % len1);
    // create a orthonormal vector to get projections to
    vector_t orthogonal = get_ortho_vec(p1, p2);

    // for projection vectors, x is min, y is max
    vector_t proj1 = find_projection(shape1, orthogonal);
    vector_t proj2 = find_projection(shape2, orthogonal);

    // they are not overlapping if the sum of the two projections are less than
    // total
    double proj1_len = proj1.y - proj1.x;
    double proj2_len = proj2.y - proj2.x;

    // MAKE SURE FMAX IS WORKING!!!!!!!!!!!
    double tot = fmax(proj1.y, proj2.y) - fmin(proj1.x, proj2.x);

    // IT DOESN'T OVERLAP!!!
    if (proj1_len + proj2_len < tot) {
      collision.collided = false;
      // collision.axis undefined
      return collision;
    }
    double overlap = fabs((tot - (proj1_len + proj2_len)));
    if (overlap < min_projection_amount) {
      min_projection_amount = overlap;
      min_projection = orthogonal;
    }
  }

  // if done and they're all still overlapping, return true for collision
  collision.collided = true;
  collision.axis = min_projection;
  return collision;
}

/**
 * Determines whether two convex polygons intersect.
 * The polygons are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding
 */
collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t collide1 = check_shape(shape1, shape2);
  collision_info_t collide2 = check_shape(shape2, shape1);

  collision_info_t collision;
  collision.collided = collide1.collided && collide2.collided;
  collision.axis = collide1.axis;
  return collision;
}