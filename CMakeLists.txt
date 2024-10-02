cmake_minimum_required(VERSION 3.5)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

SET(CMAKE_C_FLAGS "-std=c99") 

add_compile_definitions(_WIN_)

project(TigorEngine LANGUAGES C)

find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} STATIC)

include_directories(libs includes)
target_sources(${PROJECT_NAME} PRIVATE
				src/TigorEngine.c
				src/TigorGUI.c
				src/Core/e_blue_print.c
				src/Core/e_memory.c
				src/Core/e_buffer.c
				src/Core/e_camera.c
				src/Core/e_debuger.c
				src/Core/e_descriptor.c
				src/Core/e_device.c
				src/Core/e_texture.c
				src/Core/e_transform.c
				src/Core/e_window.c
				src/Core/engine.c
				src/Core/graphicsObject.c
				src/Core/pipeline.c
				src/Core/swapchain.c
				src/Data/e_resource_data.c
				src/Data/e_resource_descriptors.c
				src/Data/e_resource_engine.c
				src/Data/e_resource_shapes.c
				src/GUI/GUIManager.c
				src/GUI/e_widget.c
				src/GUI/e_widget_button.c
				src/GUI/e_widget_entry.c
				src/GUI/e_widget_combobox.c
				src/GUI/e_widget_image.c
				src/GUI/e_widget_list.c
				src/GUI/e_widget_range.c
				src/GUI/e_widget_roller.c
				src/GUI/e_widget_scroll.c
				src/GUI/e_widget_window.c
				src/Objects/e_audioObject.c
				src/Objects/gameObject.c
				src/Objects/gameObject2D.c
				src/Objects/gameObject3D.c
				src/Objects/particleSystem2D.c
				src/Objects/particleSystem3D.c
				src/Objects/marchingcube_object.c
				src/Objects/models.c
				src/Objects/primitiveObject.c
				src/Objects/render_texture.c
				src/Objects/shape_object.c
				src/Objects/sprite_object.c
				src/Tools/e_direct.c
				src/Tools/e_math.c
				src/Tools/e_shaders.c
				src/Tools/e_tools.c
				src/Tools/fbxLoader.c
				src/Tools/glTFLoader.c
				src/Tools/intersections2D.c
				src/Tools/intersections3D.c
				src/Tools/objLoader.c
				src/Tools/rayIntersections2D.c
				src/Tools/rayIntersections3D.c
				src/Tools/shader_builder.c
				src/wManager/input_manager.c
				src/wManager/posix_poll.c
				src/wManager/vulkan.c
				src/wManager/win_init.c
				src/wManager/win_monitor.c
				src/wManager/win_thread.c
				src/wManager/win_tools.c
				src/wManager/win_window.c
				src/wManager/window.c
				src/wManager/window_manager.c
				src/wManager/x11_init.c
				src/wManager/x11_monitor.c
				src/wManager/x11_tools.c
				src/wManager/x11_window.c
				libs/ufbx.c)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

enable_testing()
add_executable(TestMemory tests/memory_test.c)
target_link_libraries(TestMemory ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test1
		COMMAND TestMemory)
		
add_executable(TestCreate tests/create_object_test.c)
target_link_libraries(TestCreate ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test2
		COMMAND TestCreate)
		
add_executable(TestShaderBuilder tests/shader_builder_test.c)
target_link_libraries(TestShaderBuilder ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test3
		COMMAND TestShaderBuilder)
		
add_executable(TestRender tests/render_objets_test.c)
target_link_libraries(TestRender ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test4
		COMMAND TestRender)
		
add_executable(TestGUI tests/test_gui.c)
target_link_libraries(TestGUI ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test5
		COMMAND TestGUI)
		
add_executable(TestPath tests/file_path_test.c)
target_link_libraries(TestPath ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test6
		COMMAND TestPath)

add_executable(TestParticle tests/test_particles.c)
target_link_libraries(TestParticle ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test7
		COMMAND TestParticle)
		
add_executable(TestModel tests/test_model.c)
target_link_libraries(TestModel ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libTigorEngine.a Vulkan::Vulkan)
add_test(NAME Test8
		COMMAND TestModel)
		

