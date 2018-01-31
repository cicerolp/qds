class ShaderProgram{

    constructor(gl, vsSource, fsSource){
	this.glContext     = gl;	
	var vertexShader   = this.loadShader(gl.VERTEX_SHADER, vsSource);
	var fragmentShader = this.loadShader(gl.FRAGMENT_SHADER, fsSource);

	// Create the shader program
	this.shaderProgram = gl.createProgram();
	this.glContext.attachShader(this.shaderProgram, vertexShader);
	this.glContext.attachShader(this.shaderProgram, fragmentShader);
	this.glContext.linkProgram(this.shaderProgram);

	// If creating the shader program failed, alert
	if (!this.glContext.getProgramParameter(this.shaderProgram, gl.LINK_STATUS)) {
	    alert('Unable to initialize the shader program: ' + this.glContext.getProgramInfoLog(shaderProgram));
	    this.shaderProgram = undefined;
	}

	//
	this.uniformLocations = {};
	this.attribLocations = {};
    }

    //
    // creates a shader of the given type, uploads the source and
    // compiles it.
    //
    loadShader(type, source) {
	const shader = this.glContext.createShader(type);

	// Send the source to the shader object

	this.glContext.shaderSource(shader, source);

	// Compile the shader program

	this.glContext.compileShader(shader);

	// See if it compiled successfully

	if (!this.glContext.getShaderParameter(shader, this.glContext.COMPILE_STATUS)) {
	    alert('An error occurred compiling the shaders: ' + this.glContext.getShaderInfoLog(shader));
	    this.glContext.deleteShader(shader);
	    return undefined;
	}

	return shader;
    }

    //
    getUniformLocation(varName){
	if(this.shaderProgram == undefined){
	    alert("Error: Shader program not initialized");
	    return undefined;
	}
	else if(varName in this.uniformLocations){
	    return this.uniformLocations[varName];
	}
	else{
	    this.uniformLocations[varName] = this.glContext.getUniformLocation(this.shaderProgram, varName);
	    return this.uniformLocations[varName];
	}
    }

    //
    getAttribLocation(varName){
	if(this.shaderProgram == undefined){
	    alert("Error: Shader program not initialized");
	    return undefined;
	}
	else if(varName in this.attribLocations){
	    return this.attribLocations[varName];
	}
	else{
	    this.attribLocations[varName] = this.glContext.getAttribLocation(this.shaderProgram, varName);
	    return this.uniformLocations[varName];
	}
    }
    
    //
    bind(){
	this.glContext.useProgram(this.shaderProgram);
    }
    
    //
    release(){
	this.glContext.useProgram(null);
    }

    //
    setUniformMatrix4fv(varName,value){
	if(!(varName in this.uniformLocations)){
	    alert("Error: inexistent uniform " + varName + " in " + this.uniformLocaitons);
	    return undefined;
	}

	this.glContext.uniformMatrix4fv(
	    this.uniformLocations[varName],
	    false,
	    value);
    }

    //
    setUniformVec4fv(varName,value){
	if(!(varName in this.uniformLocations)){
	    alert("Error: inexistent uniform " + varName + " in " + this.uniformLocaitons);
	    return undefined;
	}

	this.glContext.uniform4fv(
	    this.uniformLocations[varName],
	    value);
    }

    //
    setUniform1i(varName,value){
	if(!(varName in this.uniformLocations)){
	    alert("Error: inexistent uniform " + varName + " in " + this.uniformLocaitons);
	    return undefined;
	}

	this.glContext.uniform1i(
	    this.uniformLocations[varName],
	    value);
    }

    //
    setUniform1f(varName,value){
	if(!(varName in this.uniformLocations)){
	    alert("Error: inexistent uniform " + varName + " in " + this.uniformLocaitons);
	    return undefined;
	}

	this.glContext.uniform1f(
	    this.uniformLocations[varName],
	    value);
    }
    
}
