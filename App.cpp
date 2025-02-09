// Author: Vojtěch Humpl & David Jansa

#include "App.h"


App::App() : camera(glm::vec3(0.0f, 0.0f, 3.0f)), threadPool(std::thread::hardware_concurrency()) {
	//cout << "OpenCV: " << CV_VERSION << endl;
}

void App::init(void) {
	try {
		std::cout << "Current working directory: " << std::filesystem::current_path().generic_string() << '\n';

		//if (!std::filesystem::exists("dlls"))
			//throw std::runtime_error("Directory 'dlls' not found. DLLs are expected to be there.");

		if (!std::filesystem::exists("resources"))
			throw std::runtime_error("Directory 'resources' not found. Various media files are expected to be there.");

		initOpenCV();
		initGLFW();
		initGLEW();

		initGLDebug();

		printInfoOpenCV();
		printInfoGLFW();
		printInfoGL();
		printInfoGLM();

		glfwSwapInterval(isVsyncOn ? 1 : 0);	// Enable/disable VSync

		//initTestTriangle();
		initAssets();

		glfwShowWindow(window);

		initImgui();

	} catch (const std::exception& e) {
		std::cerr << "Initialization failed: " << e.what() << std::endl;
		throw;
	}

	std::cout << "Application initialized.\n";
}

void App::initOpenCV() {
	std::cout << "OpenCV: " << CV_VERSION << std::endl;

	try {
		//open video file
		//videoCapture = cv::VideoCapture("resources/video.mkv");

		videoCapture = cv::VideoCapture(0, cv::CAP_MSMF);
		if (!videoCapture.isOpened()) {
			//throw runtime_error("Cannot open camera");
		}

		threadPool.enqueue(&App::cameraRedThreadFunction, this);
		threadPool.enqueue(&App::processingRedThreadFunction, this);
		//threadPool.enqueue(&App::cameraThreadFunction, this);
		//threadPool.enqueue(&App::processingThreadFunction, this);
		//threadPool.enqueue(&App::GUIThreadFunction, this);
		//threadPool.enqueue(&App::encodeThreadFunction, this);
	} catch (const std::exception& e) {
		std::cerr << "OpenCV init failed: " << e.what() << std::endl;
		throw;
	}
}

void App::initGLEW() {
	GLenum glew_ret;
	glew_ret = glewInit();

	if (glew_ret != GLEW_OK) {
		throw std::runtime_error(std::string("GLEW failed with error: ") + reinterpret_cast<const char*>(glewGetErrorString(glew_ret)));
	} else {
		std::cout << "GLEW: " << glewGetString(GLEW_VERSION) << std::endl;
	}

	GLenum wglew_ret = wglewInit();
	if (wglew_ret != GLEW_OK) {
		throw std::runtime_error(std::string("WGLEW failed with error: ") + reinterpret_cast<const char*>(glewGetErrorString(wglew_ret)));
	} else {
		std::cout << "WGLEW successfully initialized platform specific functions.\n";
	}

	if (!GLEW_ARB_direct_state_access)
		throw std::runtime_error("No DSA :-(");
}

void App::initGLFW() {
	glfwSetErrorCallback(GLFWErrorCallback);

	if (!glfwInit()) {
		throw std::runtime_error("GLFW failed to initialize.");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (fullscreen) {
		window = glfwCreateWindow(mode->width, mode->height, "ICP", glfwGetPrimaryMonitor(), nullptr);
	} else {
		window = glfwCreateWindow(windowWidth, windowHeight, "ICP", nullptr, nullptr);
	}

	if (!window) {
		throw std::runtime_error("GLFW window can not be created.");
	}

	windowX = (mode->width - windowWidth) / 2;
	windowY = (mode->height - windowHeight) / 2;
	glfwSetWindowPos(window, windowX, windowY);

	glfwSetWindowUserPointer(window, this);

	glfwMakeContextCurrent(window);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(window, GLFWFramebufferSizeCallback);
	glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);
	glfwSetKeyCallback(window, GLFWKeyCallback);
	glfwSetScrollCallback(window, GLFWScrollCallback);
	glfwSetCursorPosCallback(window, GLFWCursorPosCallback);
	glfwSetWindowPosCallback(window, GLFWWindowPosCallback);
}

void App::initGLDebug() {
	if (GLEW_ARB_debug_output) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageCallbackARB(MessageCallback, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	} else {
		std::cerr << "OpenGL debug output not supported.\n";
	}
}

void App::initImgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	std::cout << "ImGUI version: " << ImGui::GetVersion() << "\n";
}

void App::initAssets() {
	// shaders
	ShaderProgram modelShader("modelVS.glsl", "modelFS.glsl");
	shaders.push_back(std::move(modelShader));

	// models
	Model* rabbitModel = new Model("resources/bunny10k_textured.obj", shaders[0], true);
	rabbitModel->origin = glm::vec3(0.0f, 0.0f, 0.0f);
	rabbitModel->orientation = glm::vec3(0.0f, 0.0f, 0.0f);
	Entity* rabbit = new Entity(rabbitModel, nullptr, glm::vec3(0.0f, 0.0f, 0.0f));
	entities.push_back(rabbit);
	//models.push_back(std::move(testModel));

	Model* grid = new Model(Assets::createGrid(10, shaders[0]));
	Entity* gridEntity = new Entity (grid, nullptr, glm::vec3(0.0f, 0.0f, 0.0f));
	entities.push_back(gridEntity);
	//models.push_back(std::move(grid));

	//Model terrain = Assets::createTerrain(100, 15.f, 0.01f, shaders[0]);
	//models.push_back(std::move(terrain));
	TerrainEntity* terrain = new TerrainEntity(100, 15.f, 0.01f, shaders[0]);
	entities.push_back(terrain);

	Model* cube = new Model(Assets::createCube(2.0f, glm::vec4(0.89f, 0.85f, 0.173f, 1.0f), shaders[0]));
	//cube.origin = glm::vec3(10.0f, 1.0f, 0.0f);
	BoxCollider* boxCollider = new BoxCollider(cube->origin, glm::vec3(2.0f));
	gCollisionManager.addCollider(boxCollider);
	Entity* cubeEntity = new Entity(cube, boxCollider, glm::vec3(10.0f, -2.0f, 0.0f), glm::vec3(2.0f));
	entities.push_back(cubeEntity);
	//models.push_back(std::move(cube));

	Model* sphere = new Model(Assets::createSphere(1.0f, 20, 20, glm::vec4(1, 0, 0, 1), shaders[0]));
	SphereCollider* sphereCollider = new SphereCollider(sphere->origin, 1.0f);
	gCollisionManager.addCollider(sphereCollider);
	Entity* sphereEntity = new Entity(sphere, sphereCollider, glm::vec3(10.0f, -2.0f, 2.0f), glm::vec3(1.0f));
	entities.push_back(sphereEntity);

	Model* sub = new Model("resources/sub.obj", shaders[0], true);
	Entity* subEntity = new Entity(sub, nullptr, glm::vec3(10.0f, 20.0f, 10.0f));
	entities.push_back(subEntity);

	Model* skull = new Model("resources/skull.obj", shaders[0], true);
	Entity* skullEntity = new Entity(skull, nullptr, glm::vec3(0.0f, 30.0f, -50.0f));
	skullEntity->orientation = glm::vec3(-90.0f, 0.0f, 0.0f);
	entities.push_back(skullEntity);

	Model* cube2 = new Model(Assets::createCube(2.0f, glm::vec4(0.89f, 0.169f, 0.792f, 0.4f), shaders[0]));
	BoxCollider* boxCollider2 = new BoxCollider(cube2->origin, glm::vec3(2.0f));
	gCollisionManager.addCollider(boxCollider2);
	Entity* cubeEntity2 = new Entity(cube2, boxCollider2, glm::vec3(10.0f, 0.0f, 20.0f), glm::vec3(2.0f));
	entities.push_back(cubeEntity2);

	Model* cube3 = new Model(Assets::createCube(2.0f, glm::vec4(0.314f, 0.525f, 0.831f, 0.4f), shaders[0]));
	BoxCollider* boxCollider3 = new BoxCollider(cube3->origin, glm::vec3(2.0f));
	gCollisionManager.addCollider(boxCollider3);
	Entity* cubeEntity3 = new Entity(cube3, boxCollider3, glm::vec3(10.0f, 0.0f, 23.0f), glm::vec3(2.0f));
	entities.push_back(cubeEntity3);

	//sunLight = Assets::createDirectionalLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//pointLight = Assets::createPointLight(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	//spotLight = Assets::createSpotLight(glm::vec3(10.0f, 20.0f, 20.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));
	//movingSpotLight = Assets::createSpotLight(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));

	lightsBlock.numLights = 4;
	// sunlight
	lightsBlock.lights[0].type = 1;	// 0 == disabled, 1 == directional, 2 == point, 3 == spot
	lightsBlock.lights[0].direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	lightsBlock.lights[0].padding2 = 0.0f;
	lightsBlock.lights[0].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	lightsBlock.lights[0].diffuse = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	lightsBlock.lights[0].specular = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	lightsBlock.lights[0].constant = 1.0f;
	lightsBlock.lights[0].linear = 0.0f;
	lightsBlock.lights[0].quadratic = 0.0f;
	lightsBlock.lights[0].cutOff = 0.0f;
	lightsBlock.lights[0].outerCutOff = 0.0f;

	// point light
	lightsBlock.lights[1].type = 2;
	lightsBlock.lights[1].position = glm::vec3(0.0f, 20.0f, 0.0f);
	lightsBlock.lights[1].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	lightsBlock.lights[1].diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	lightsBlock.lights[1].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightsBlock.lights[1].constant = 1.0f;
	lightsBlock.lights[1].linear = 0.09f;
	lightsBlock.lights[1].quadratic = 0.032f;
	lightsBlock.lights[1].cutOff = 0.0f;
	lightsBlock.lights[1].outerCutOff = 0.0f;

	// spot light
	lightsBlock.lights[2].type = 3;
	lightsBlock.lights[2].position = glm::vec3(10.0f, 20.0f, 20.0f);
	lightsBlock.lights[2].direction = glm::vec3(0.0f, -1.0f, 0.0f);
	lightsBlock.lights[2].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	lightsBlock.lights[2].diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	lightsBlock.lights[2].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightsBlock.lights[2].constant = 1.0f;
	lightsBlock.lights[2].linear = 0.09f;
	lightsBlock.lights[2].quadratic = 0.032f;
	lightsBlock.lights[2].cutOff = glm::cos(glm::radians(12.5f));
	lightsBlock.lights[2].outerCutOff = glm::cos(glm::radians(15.0f));

	// moving point light
	lightsBlock.lights[3].type = 3;
	lightsBlock.lights[3].position = glm::vec3(0.0f, 20.0f, 0.0f);
	lightsBlock.lights[3].direction = glm::vec3(0.0f, -1.0f, 0.0f);
	lightsBlock.lights[3].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	lightsBlock.lights[3].diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	lightsBlock.lights[3].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightsBlock.lights[3].constant = 1.0f;
	lightsBlock.lights[3].linear = 0.09f;
	lightsBlock.lights[3].quadratic = 0.032f;
	lightsBlock.lights[3].cutOff = 0.0f;
	lightsBlock.lights[3].outerCutOff = 0.0f;
	lightsBlock.lights[3].cutOff = glm::cos(glm::radians(12.5f));
	lightsBlock.lights[3].outerCutOff = glm::cos(glm::radians(15.0f));

	player = new Player(shaders[0], glm::vec3(0.0f, 5.0f, 0.0f));
	player->affectedByGravity = true;
	physicsEntities.push_back(player);
}

int App::run(void) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenBuffers(1, &lightsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightsBlock), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	double lastFrameTime = glfwGetTime();
	double fps_last_displayed = lastFrameTime;
	int fps_counter_frames = 0;
	double FPS = 0.0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	std::cout << "Size of Light in C++: " << sizeof(Light) << std::endl;
	std::cout << "Size of LightsBlock in C++: " << sizeof(LightsBlock) << std::endl;

	int squareCorner = 0;
	float cube1Alpha = 0.5f;
	float cube2Alpha = 0.5f;
	while (!glfwWindowShouldClose(window)) {
		double now = glfwGetTime();
		deltaTime = now - lastFrameTime;
		lastFrameTime = now;

		processInput(deltaTime);

		//entities[5]->moveTowards(glm::vec3(100.0f, 20.0f, 100.0f), 20.0f, deltaTime);

		// submarine
		if (squareCorner == 0) {
			if(entities[5]->moveTowards(glm::vec3(100.0f, 20.0f, 100.0f), 25.0f, deltaTime)) {
				squareCorner = 1;
			}
		} else if (squareCorner == 1) {
			if (entities[5]->moveTowards(glm::vec3(100.0f, 20.0f, 0.0f), 25.0f, deltaTime)) {
				squareCorner = 2;
			}
		} else if (squareCorner == 2) {
			if (entities[5]->moveTowards(glm::vec3(0.0f, 20.0f, 0.0f), 25.0f, deltaTime)) {
				squareCorner = 3;
			}
		} else if (squareCorner == 3) {
			if (entities[5]->moveTowards(glm::vec3(0.0f, 20.0f, 100.0f), 25.0f, deltaTime)) {
				squareCorner = 0;
			}
		}

		entities[0]->moveInCircle(glm::vec3(0.0f, 0.0f, 0.0f), 10.0f, 0.5f, now);

		float radius = 10.0f;
		lightsBlock.lights[3].position = glm::vec3(radius * cos(now), 20.0f, radius * sin(now));
		
		glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsBlock), &lightsBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		for (auto& entity : physicsEntities) {
			entity->update(deltaTime);
		}

		for (auto& entity : entities) {
			entity->update(deltaTime);
		}

		for (auto it = ParticleSystem::particles.begin(); it != ParticleSystem::particles.end();) {
			ParticleEntity* p = *it;
			p->update(deltaTime);
			if (p->lifetime <= 0.0f) {
				delete* it;
				it = ParticleSystem::particles.erase(it);
			} else {
				++it;
			}
		}

		if (showImgui) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowPos(ImVec2(10, 10));
			ImGui::SetNextWindowSize(ImVec2(250, 220));
			ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("V-Sync: %s", isVsyncOn ? "ON" : "OFF");
			ImGui::Text("FPS: %.1f", FPS);
			ImGui::Text("Camera position: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z);
			ImGui::Text("Red detected: %s", redDetected.load(std::memory_order_relaxed) ? "YES" : "NO");
			ImGui::SliderFloat("Cube1 alpha", &cube1Alpha, 0.0f, 1.0f);
			ImGui::SliderFloat("Cube2 alpha", &cube2Alpha, 0.0f, 1.0f);
			ImGui::Text("(press RMB to release mouse)");
			ImGui::Text("(press I to show/hide info)");
			ImGui::Text("(press G to detach/attach camera)");
			ImGui::End();
		}

		entities[7]->setAlpha(cube1Alpha);
		entities[8]->setAlpha(cube2Alpha);

		//double time_speed = showImgui ? 0.0 : 1.0;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = camera.getProjectionMatrix((float)windowWidth / (float)windowHeight, 0.01f, 1000.0f);
		glm::mat4 view = camera.getViewMatrix();

		shaders[0].activate();
		shaders[0].setUniform("projection", projection);
		shaders[0].setUniform("view", view);
		shaders[0].setUniform("viewPos", camera.position);


		shaders[0].setUniform("ambientOcclusion", 0.2f);


		if (player) {
			player->draw();
		}
		
		for (auto& entity : entities) {
			if (entity->transparent)
				transparentEntities.push_back(entity);
			else
				opaqueEntities.push_back(entity);
		}

		for (auto& entity : opaqueEntities) {
			entity->draw();
		}

		for (auto& particle : ParticleSystem::particles) {
			particle->draw();
		}

		std::sort(transparentEntities.begin(), transparentEntities.end(),
			[&](Entity* a, Entity* b) {
				float distA = glm::distance2(a->position, camera.position);
				float distB = glm::distance2(b->position, camera.position);
				return distA > distB; // want farthest first
			}
		);

		glDepthMask(GL_FALSE);
		for (auto& entity : transparentEntities) {
			entity->draw();
		}
		glDepthMask(GL_TRUE);

		transparentEntities.clear();
		opaqueEntities.clear();


		if (showImgui) {
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}


		glfwSwapBuffers(window);
		glfwPollEvents();


		fps_counter_frames++;
		if (now - fps_last_displayed >= 1.0) {
			FPS = fps_counter_frames / (now - fps_last_displayed);
			fps_last_displayed = now;
			fps_counter_frames = 0;
			std::cout << "\r[FPS] " << FPS << "     " << std::flush;
		}
	}

	return EXIT_SUCCESS;
}

void App::processInput(float deltaTime) {
	glm::vec3 direction(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		direction += camera.front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		direction -= camera.front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		direction -= camera.right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		direction += camera.right;


	if (cameraDetached) {
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			direction += camera.worldUp;
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
			direction -= camera.worldUp;
	}

	// normalize so diagonal movement isnt faster
	if (glm::length(direction) > 0.0f)
		direction = glm::normalize(direction);

	float speedMultiplier = 1.0f;
	if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) || redDetected.load(std::memory_order_relaxed)) {
		speedMultiplier = 2.5f;
	}

	if (cameraDetached) {
		// detached: move the camera directly
		camera.position += direction * camera.movementSpeed * deltaTime * speedMultiplier;
	} else {
		// attached: update the players position
		if (player) {
			glm::vec3 accel = direction * player->movementAcceleration * speedMultiplier;
			accel.y = 0.0f;
			player->velocity += accel * deltaTime;

			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				if (player->isOnGround) {
					player->velocity.y = player->jumpVelocity;
					player->isOnGround = false;
				}
			}

			float behindDistance = 1.0f;
			float heightOffset = 0.5f;

			glm::vec3 headPos = player->getHeadPosition();
			glm::vec3 behindPos = headPos - camera.front * behindDistance;
			behindPos.y += heightOffset;

			camera.position = behindPos;
		}
	}
}

void App::cameraRedThreadFunction() {
	cv::Mat frame;
	while (!stopSignal) {
		videoCapture.read(frame);
		if (frame.empty() && displayQueue.empty()) {
			std::cerr << "Camera disconnected or end of stream.\n";
			stopSignal = true;
			break;
		}
		frameQueue.push(frame);
	}
	frameQueue.stop();
}

void App::processingRedThreadFunction() {
	cv::Mat frame;
	while (!stopSignal) {
		if (frameQueue.pop(frame)) {
			if (frame.empty()) {
				continue;
			}
			bool isRed = findRed(frame);
			// save to atomic
			redDetected.store(isRed, std::memory_order_relaxed);
		}
	}
}

bool App::findRed(const cv::Mat& img) {
	cv::Mat hsv;
	cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

	cv::Mat mask1, mask2;
	cv::inRange(hsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), mask1);
	cv::inRange(hsv, cv::Scalar(170, 100, 100), cv::Scalar(180, 255, 255), mask2);

	cv::Mat redMask = mask1 | mask2;
	return cv::countNonZero(redMask) > 0;
}


void App::cameraThreadFunction() {
	cv::Mat frame;

	while (!stopSignal) {
		auto start = std::chrono::high_resolution_clock::now();

		videoCapture.read(frame);
		if (frame.empty() && displayQueue.empty()) {
			std::cerr << "Camera disconnected or end of stream.\n";
			stopSignal = true;
			break;
		}
		frameQueue.push(frame);

		displayQueue.push(std::make_tuple(frame, "Original Frame"));
		encodeQueue.push(frame);

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_microseconds = end - start;
		auto ms = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
		//cout << "Elapsed time capturing: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;
	}
}

void App::encodeThreadFunction() {
	cv::Mat frame;

	float target_coefficient = 0.5f;
	while (!stopSignal) {
		if (encodeQueue.pop(frame)) {
			if (frame.empty()) {
				continue;
			}

			auto start = std::chrono::high_resolution_clock::now();

			auto size_uncompressed = frame.elemSize() * frame.total();
			auto size_compressed_limit = size_uncompressed * target_coefficient;
			std::vector<uchar> bytes = lossyLimitBW(frame, size_compressed_limit);
			//vector<uchar> bytes = lossyLimitQuality(frame, 30.0f);

			decodeQueue.push(bytes);

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed_microseconds = end - start;
			auto ms = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
			//cout << "Elapsed time encoding: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;
		}
	}
}

void App::processingThreadFunction() {
	cv::Mat frame;

	while (!stopSignal) {
		if (frameQueue.pop(frame)) {
			if (frame.empty()) {
				continue;
			}

			auto start = std::chrono::high_resolution_clock::now();

			cv::Point2f center = findObject(frame);
			cv::Point2f center_normalized(center.x / frame.cols, center.y / frame.rows);

			cv::Mat scene_cross;
			frame.copyTo(scene_cross);
			drawCrossNormalized(scene_cross, center_normalized, 30);

			displayQueue.push(std::make_tuple(scene_cross, "Processed Frame"));

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed_microseconds = end - start;
			auto ms = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
			//cout << "Elapsed time processing: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;
		}
	}
}

void App::GUIThreadFunction() {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		stopSignal = true;
		return;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Window", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		stopSignal = true;
		return;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(1);

	cv::Mat frame;
	std::tuple<cv::Mat, std::string> display;
	//int target_fps = 15;

	while (!stopSignal) {
		if (displayQueue.pop(display)) {
			frame = std::get<0>(display);
			std::string window_name = std::get<1>(display);
			if (frame.empty()) {
				continue;
			}

			cv::imshow(window_name, frame);

			if (cv::pollKey() == 27) {
				stopSignal = true;
				break;
			}

			//int ms_per_frame = 1000.0 / target_fps;
			//int key = cv::waitKey(ms_per_frame);
		}
	}
}

std::vector<uchar> App::lossyLimitBW(cv::Mat& input_img, size_t size_limit) {
	std::string suff(".jpg"); // target format
	if (!cv::haveImageWriter(suff))
		throw std::runtime_error("Can not compress to format:" + suff);

	std::vector<uchar> bytes;
	std::vector<int> compression_params;

	// prepare parameters for JPEG compressor
	// we use only quality, but other parameters are available (progressive, optimization...)
	std::vector<int> compression_params_template;
	compression_params_template.push_back(cv::IMWRITE_JPEG_QUALITY);

	std::cout << '[';

	//try step-by-step to decrease quality by 5%, until it fits into limit
	for (auto i = 100; i > 0; i -= 5) {
		compression_params = compression_params_template; // reset parameters
		compression_params.push_back(i);                  // set desired quality
		std::cout << i << ',';

		// try to encode
		cv::imencode(suff, input_img, bytes, compression_params);

		// check the size limit
		if (bytes.size() <= size_limit)
			break; // ok, done 
	}
	std::cout << "]\n";

	return bytes;
}

std::vector<uchar> App::lossyLimitQuality(cv::Mat& input_img, float target_quality) {
	std::string suff(".jpg"); // Target format
	if (!cv::haveImageWriter(suff))
		throw std::runtime_error("Cannot compress to format: " + suff);

	std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, 95 };

	std::vector<uchar> best_bytes;
	int best_quality = -1;

	std::vector<int> qualities(100);
	std::iota(qualities.begin(), qualities.end(), 1);

	std::vector<std::future<void>> futures;
	std::mutex mtx;

	auto compress_and_evaluate_task = [&](int quality) {
		std::vector<uchar> bytes;
		std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, quality };
		cv::imencode(suff, input_img, bytes, params);

		cv::Mat decoded_img = cv::imdecode(bytes, cv::IMREAD_COLOR);

		// Compute PSNR between the original and compressed images
		double psnr = cv::PSNR(input_img, decoded_img);

		std::cout << "Quality: " << quality << ", PSNR: " << psnr << std::endl;

		// Update best quality if criteria met
		if (psnr >= target_quality) {
			std::lock_guard<std::mutex> lock(mtx);
			if (best_quality == -1 || quality < best_quality) {
				best_quality = quality;
				best_bytes = std::move(bytes); // Save the best compressed image
			}
		}
	};

	for (int quality : qualities) {
		futures.emplace_back(threadPool.enqueue(compress_and_evaluate_task, quality));
	}

	for (auto& future : futures) {
		future.get();
	}

	if (best_quality == -1) {
		throw std::runtime_error("Cannot achieve target PSNR with available quality settings.");
	}

	std::cout << "Selected Quality: " << best_quality << std::endl;
	return best_bytes;
}

cv::Mat App::threshold(const cv::Mat& img, const double h_low, const double s_low, const double v_low, const double h_hi, const double s_hi, const double v_hi) {
	cv::Mat img_hsv, img_threshold;

	cv::cvtColor(img, img_hsv, cv::COLOR_BGR2HSV);

	cv::Scalar lower_threshold = cv::Scalar(h_low, s_low, v_low);
	cv::Scalar upper_threshold = cv::Scalar(h_hi, s_hi, v_hi);
	cv::inRange(img_hsv, lower_threshold, upper_threshold, img_threshold);

	return img_threshold;
}

cv::Point2f App::findObject(const cv::Mat& img) {
	// First range for red hue
	cv::Mat img_threshold1 = threshold(img, 0, 100, 100, 10, 255, 255);

	// Second range for red hue
	cv::Mat img_threshold2 = threshold(img, 170, 100, 100, 180, 255, 255);

	// Combine both ranges
	cv::Mat img_threshold;
	cv::bitwise_or(img_threshold1, img_threshold2, img_threshold);

	// find contours
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(img_threshold, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	// find the largest contour
	int largest_area = 0;
	int largest_contour_index = 0;
	for (size_t i = 0; i < contours.size(); i++) {
		double a = cv::contourArea(contours[i], false);
		if (a > largest_area) {
			largest_area = a;
			largest_contour_index = i;
		}
	}

	if (contours.empty())
		return cv::Point2f(0, 0);

	// find the center of the largest contour
	cv::Moments mu = cv::moments(contours[largest_contour_index]);
	cv::Point2f center(mu.m10 / mu.m00, mu.m01 / mu.m00);

	// if center nan, return (0,0)
	if (center.x != center.x || center.y != center.y)
		return cv::Point2f(0, 0);

	return center;
}

void App::drawCross(cv::Mat& img, int x, int y, int size) {
	cv::Point p1(x - size / 2, y);
	cv::Point p2(x + size / 2, y);
	cv::Point p3(x, y - size / 2);
	cv::Point p4(x, y + size / 2);

	cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
	cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

void App::drawCrossNormalized(cv::Mat& img, cv::Point2f center_normalized, int size) {
	center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
	center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
	size = std::clamp(size, 1, std::min(img.cols, img.rows));

	cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

	drawCross(img, center_absolute.x, center_absolute.y, size);
}

void App::printInfoGLM() {
	std::cout << "GLM version: " << GLM_VERSION_MAJOR << '.' << GLM_VERSION_MINOR << '.' << GLM_VERSION_PATCH << "rev" << GLM_VERSION_REVISION << std::endl;
}

void App::printInfoOpenCV() {
	std::cout << " Capture source: " <<
		": width=" << videoCapture.get(cv::CAP_PROP_FRAME_WIDTH) <<
		", height=" << videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

void App::printInfoGLFW(void) {
	int major, minor, revision;
	glfwGetVersion(&major, &minor, &revision);
	std::cout << "Running GLFW DLL " << major << '.' << minor << '.' << revision << '\n';
	std::cout << "Compiled against GLFW "
		<< GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION
		<< '\n';
}

void App::printInfoGL() {
	auto vendor_s = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	std::cout << "OpenGL driver vendor: " << (vendor_s ? vendor_s : "UNKNOWN") << '\n';

	auto renderer_s = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	std::cout << "OpenGL renderer: " << (renderer_s ? renderer_s : "<UNKNOWN>") << '\n';

	auto version_s = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	std::cout << "OpenGL version: " << (version_s ? version_s : "<UNKNOWN>") << '\n';

	auto glsl_s = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	std::cout << "Primary GLSL shading language version: " << (glsl_s ? glsl_s : "<UNKNOWN>") << std::endl;

	// get GL profile info
	{
		GLint profile_flags;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_flags);
		std::cout << "Current profile: ";
		if (profile_flags & GL_CONTEXT_CORE_PROFILE_BIT)
			std::cout << "CORE";
		else
			std::cout << "COMPATIBILITY";
		std::cout << std::endl;
	}

	// get context flags
	{
		GLint context_flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
		std::cout << "Active context flags: ";
		if (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
			std::cout << "GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
			std::cout << "GL_CONTEXT_FLAG_DEBUG_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
			std::cout << "GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
			std::cout << "GL_CONTEXT_FLAG_NO_ERROR_BIT";
		std::cout << std::endl;
	}

	{ // get extension list
		GLint n = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		std::cout << "GL extensions: " << n << '\n';

		//for (GLint i = 0; i < n; i++) {
		//    const char* extension_name = (const char*)glGetStringi(GL_EXTENSIONS, i);
		//    std::cout << extension_name << '\n';
		//}
	}
}

void App::destroy(void) {
	stopSignal = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	frameQueue.clear();
	displayQueue.clear();
	encodeQueue.clear();
	decodeQueue.clear();

	// clean up ImGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// clean up OpenCV
	cv::destroyAllWindows();
	if (videoCapture.isOpened())
		videoCapture.release();

	// clean-up GLFW
	if (window) {
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();

	for (auto& entity : entities) {
		delete entity;
	}

	for (auto& entity : physicsEntities) {
		delete entity;
	}

	gAudioPlayer.cleanFinishedSounds();
}

App::~App() {
	destroy();

	std::cout << "Application exited cleanly.\n";
}
