-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25, 0, 1, 0, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0, 0, 1, 0, 0)
blue = gr.material({0.7, 0.6, 1}, {0.5, 0.4, 0.8}, 25, 0, 1, 0, 0)
mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25, 0.0, 1.0, 0, 0)

scene = gr.node('scene')
scene:rotate('X', 30)
scene:translate(6, -2, -30)

-- the arc
arc = gr.node('arc')
scene:add_child(arc)
arc:translate(0,0,-10)
arc:rotate('Y', 100)



c1 = gr.cylinder('c1')
arc:add_child(c1)
c1:set_material(gold)
c1:scale(2, 5, 2)
c1:translate(0, 0, 0)

c2 = gr.cone('c2')
arc:add_child(c2)
c2:set_material(grass)
c2:scale(2, 6, 2)
c2:translate(10, 0, 0)
c2:rotate('X', 10)
c2:rotate('Y', 20)
c2:rotate('Z', 30)

c3 = gr.cylinder('c3')
arc:add_child(c3)
c3:set_material(blue)
c3:scale(2, 8, 3)
c3:translate(0, 0, 10)


-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(mat1)
plane:scale(30, 30, 30)

-- The lights
l1 = gr.light({200,200,400}, {0.8, 0.8, 0.8}, {1, 0, 0}, 0)
l2 = gr.light({0, 5, -20}, {0.4, 0.4, 0.8}, {1, 0, 0}, 0)

gr.render(scene, 'add_primitives.png', 256, 256,
	  {0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1, l2}, 0, 1, {})
