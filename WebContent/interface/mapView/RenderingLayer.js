class RenderingLayer{
    constructor(gl,shaderManager,minZoomLevel,maxZoomLevel){
	//
	this.glContext     = gl;
	this.dataInfo      = {};
	
	//
	this.vertexBuffer    = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.shaderManager   = shaderManager;
	this.dataInitialized = false;
	this.enabled         = true;
    }

    setVertexBufferSize(numElements,numComponents){
	var totalSize = numElements * numComponents;
	var foo = Array(totalSize).fill(0);
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(foo,this.glContext.STREAM_DRAW);
	this.vertexBuffer.release();
    }
    
    setEnabled(v){
	this.enabled = v;
    }

    setData(newData){
    }

    renderData(){
    }
}
