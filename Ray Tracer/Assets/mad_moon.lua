-- mad moon

refract_moon = gr.material({0.3, 0.3, 0.4}, {0.2, 0.2, 0.4}, 5, 0.0, 1.5, 0, 0.0)
reflect_moon = gr.material({0.8, 0.5, 0.8}, {0.7, 0.7, 0.7}, 50, 0.4, 1.0, 0, 0.0)
wood_moon = gr.material({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 25, 0.0, 1.0, 1, 0.3)
leaf_moon = gr.material({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 20, 0.0, 1.0, 3, 0.0)
hill = gr.material({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 0, 0, 1.0, 4, 0.8)
blue = gr.material({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 25, 0, 1.0, 2, 0)
stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0, 0, 1.0, 0, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20, 0, 1.0, 3, 0.5)


scene = gr.node('scene')
scene:rotate('X', 30)

-- the con
con = gr.node('con')
scene:add_child(con)

mad_moon = gr.sphere('mad_moon')
con:add_child(mad_moon)
mad_moon:set_material(reflect_moon)
mad_moon:scale(10, 10, 10)
mad_moon:translate(0, 20, 0)


tree_moon = gr.sphere('tree_moon')
con:add_child(tree_moon)
tree_moon:set_material(wood_moon)
tree_moon:scale(3, 3, 3)
tree_moon:translate(12, 10, 0)

brick_moon = gr.sphere('brick_moon')
con:add_child(brick_moon)
brick_moon:set_material(blue)
brick_moon:scale(2.5, 2.7, 2.3)
brick_moon:translate(-15, 5, -7)

c1 = gr.cylinder('c1')
con:add_child(c1)
c1:set_material(hide)
c1:scale(1.5, 14.5, 1.5)
c1:translate(3, 0, 10)

pure_moon = gr.sphere('pure_moon')
con:add_child(pure_moon)
pure_moon:set_material(refract_moon)
pure_moon:scale(2, 2, 2)
pure_moon:translate(2, 12, 20)

tech_moon = gr.sphere('tech_moon')
con:add_child(tech_moon)
tech_moon:set_material(leaf_moon)
tech_moon:scale(2, 2, 2)
tech_moon:translate(-8, 2, 0)


-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(hill)
plane:scale(100, 55, 150)
plane:translate(0, 0, -80)



-- The lights
l1 = gr.light({200,200,400}, {0.8, 0.8, 0.8}, {1, 0, 0}, 1)
l2 = gr.light({0, 100, -20}, {0.4, 0.4, 0.8}, {1, 0, 0}, 0)

gr.render(scene, 'mad_moon.png', 512, 512,
	  {0, 0, 40,}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.2, 0.2, 0.2}, {l1, l2}, 0, 1, {'texture1.png', 'texture2.png', 'texture4.png', 'texture3.png'})
