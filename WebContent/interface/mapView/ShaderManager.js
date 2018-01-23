class ShaderManager{

    constructor(gl){
	//
	this.glContext = gl;
	this.shaders = {};
	
	//
	this.initShaders();
    }

    initShaders(){
	var myShader = undefined;

	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["modelViewShader"],shaderSources["fragment"]["constantColor"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uModelViewMatrix');
	this.shaders["constantColor"] = myShader; 

	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["modelViewShader"],shaderSources["fragment"]["uniformColor"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uModelViewMatrix');
	myShader.getUniformLocation('uPolColor');
	this.shaders["uniformColor"] = myShader;

	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["pointShader"],shaderSources["fragment"]["constantColor"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uModelViewMatrix');
	this.shaders["pointShader"] = myShader;

	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["latLngShader"],shaderSources["fragment"]["constantColor"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getAttribLocation('aTextureCoordinates');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uModelViewMatrix');
	this.shaders["latLngConstColShader"] = myShader;

	
	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["latLngShader"],shaderSources["fragment"]["circlePointTextureColor"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getAttribLocation('aTextureCoordinates');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uColormapTexture');
	this.shaders["latLngPointShader"] = myShader;

	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["latLngPickingShader"],shaderSources["fragment"]["circlePointPicking"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getAttribLocation('aTextureCoordinates');
	myShader.getAttribLocation('aColorIndices');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uColormapTexture');
	myShader.getUniformLocation('uRenderPicking');
	myShader.getUniformLocation('uColorMode');
	this.shaders["latLngPickingPointShader"] = myShader;

	//
	myShader = new ShaderProgram(this.glContext,shaderSources["vertex"]["latLngShader"],shaderSources["fragment"]["textureColor"]);
	myShader.getAttribLocation('aVertexPosition');
	myShader.getAttribLocation('aTextureCoordinates');
	myShader.getUniformLocation('uProjectionMatrix');
	myShader.getUniformLocation('uColormapTexture');
	myShader.getUniformLocation('uMinNormalizer');
	myShader.getUniformLocation('uMaxNormalizer');
	myShader.getUniformLocation('uOpacity');
	this.shaders["latLngShader"] = myShader;
    }

    getShader(shaderName){
	if(!(shaderName in this.shaders)){
	    alert("Error (Shader Manager): Shader" + shaderName + " does not exist!");
	    return undefined;
	}

	return this.shaders[shaderName];
    }
}
