-- A simple scene with some miscellaneous geometry.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25, 0.0, 1.0, 0, 0)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25, 0.0, 1.0, 0, 0)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25, 0, 1.0, 0, 0)


scene_root = gr.node('root')

s1 = gr.nh_sphere('s1', {-100, -100, 300}, 100)
scene_root:add_child(s1)
s1:set_material(mat1)

s2 = gr.nh_sphere('s2', {0, 0, 0}, 100)
scene_root:add_child(s2)
s2:set_material(mat2)


s3 = gr.nh_sphere('s3', {100, 100, -300}, 100)
scene_root:add_child(s3)
s3:set_material(mat3)



white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0}, 0)
orange_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0}, 0)

gr.render(scene_root, 'dof_ball2.png', 300, 300,
	  {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, orange_light}, 800, 1, {})
