import fried.Application;
import fried.Project;
import fried.Window;
import fried.graphics.Color;
import fried.graphics.Texture;
import fried.input.Input;
import fried.input.Key;
import fried.io.Assets;
import fried.scene.GameObject;
import fried.scene.Scene;
import fried.scene.Sprite;

class Main {
	static inline var SQUARE_SIZE = 160;

	public static function main():Void {
		Application.init();

		var window = new Window(Project.windowTitle(), 960, 540);
		window.setIcon(Assets.game("icon.png"));
		window.onClose = function() {
			Application.quit();
		};

		var renderer = Application.createRenderer(window);
		renderer.drawColor = Color.rgb(24, 24, 32);

		var squareTexture = Texture.load(renderer, Assets.game("white.png"));

		var scene = new Scene("Main");

		var square = scene.add(new GameObject("Square", (renderer.width - SQUARE_SIZE) / 2, (renderer.height - SQUARE_SIZE) / 2));
		square.transform.setScale(SQUARE_SIZE / squareTexture.width);
		square.addComponent(new Sprite(squareTexture));

		Application.run(function() {
			if (Input.isKeyDown(Key.Escape)) {
				Application.quit();
			}

			scene.update();
			scene.draw();
		});

		scene.destroy();
		squareTexture.destroy();
		Application.destroyRenderer();
		window.destroy();
		Application.shutdown();
	}
}
