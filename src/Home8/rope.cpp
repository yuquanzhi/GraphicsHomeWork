#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {
	//阻尼系数
	const float damping_factor = 0.00005f;
    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        //计算对应的节点位置
		Vector2D direction = (end - start).unit();
        //每个节点之间的距离
		float distance = (end - start).norm() / (num_nodes - 1);
		//根据节点数目创建节点 并在两个节点之间创建弹簧
		for (int i = 0; i < num_nodes; ++i) {
			Vector2D position = start + direction * distance * i;
			masses.push_back(new Mass(position, node_mass, false));
		}
		//创建弹簧
		for (int i = 0; i < num_nodes - 1; ++i) {
			springs.push_back(new Spring(masses[i], masses[i + 1], k));
		}
		// TODO (Part 1): Pin the nodes specified in `pinned_nodes`
		for (int i : pinned_nodes) {
			if (i >= 0 && i < num_nodes) {
				masses[i]->pinned = true;
			}
		}



//        Comment-in this part when you implement the constructor
//        for (auto &i : pinned_nodes) {
//            masses[i]->pinned = true;
//        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node

			Vector2D direction = s->m2->position - s->m1->position;
            //计算对应的方向
			Vector2D force = direction.unit() * s->k * (direction.norm() - s->rest_length);
			//将力添加到对应的节点上
			s->m1->forces += force;
			s->m2->forces -= force; // 作用力与反作用力

        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                //对节点添加重力
				m->forces += gravity * m->mass;

				//计算新的速度
				m->velocity += (m->forces / m->mass) * delta_t;
				//计算新的位置
				m->position += m->velocity * delta_t;
                // TODO (Part 2): Add global damping





            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
			Vector2D direction = s->m2->position - s->m1->position;

			//计算对应的方向
			Vector2D force = direction.unit() * s->k * (direction.norm() - s->rest_length);
			//将力添加到对应的节点上
			s->m1->forces += force;
			s->m2->forces -= force; // 作用力与反作用力

			// TODO (Part 3): Add the force due to gravity, then compute the new position

			s->m1->forces += gravity * s->m1->mass;
			s->m2->forces += gravity * s->m2->mass;
			

        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
				m->position += (1- damping_factor)*(m->position - m->last_position) + (m->forces / m->mass) * delta_t * delta_t;
				// TODO (Part 3.2): Set the last position of the rope mass
				m->last_position = temp_position;
				// TODO (Part 3.3): Reset forces on the rope mass
				m->forces = Vector2D(0, 0);
				// TODO (Part 3.4): Add global Verlet damping
				// Verlet damping is typically applied by scaling the velocity
				Vector2D velocity = m->position - m->last_position;
                
				m->position -= velocity * 0.01; // Adjust the damping factor as needed
				// Note: The damping factor can be adjusted based on the desired effect.
				




				// TODO (Part 3): Add global Verlet damping

				// Verlet damping is typically applied by scaling the velocity




                // TODO (Part 4): Add global Verlet damping
            }
        }
    }
}
