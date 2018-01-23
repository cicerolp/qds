class FramebufferObject{

    constructor(gl,width,height){
	this.glContext = gl;
	//create texture to render to
	
	
	//create gl fbo
	this.fbo = this.glContext.createFramebuffer();
	this.glContext.bindFramebuffer(this.glContext.FRAMEBUFFER, this.fbo);
	this.fbo.width  = width;
	this.fbo.height = height;

	//
	this.renderTexture = this.glContext.createTexture();
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.renderTexture);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MAG_FILTER, this.glContext.LINEAR);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MIN_FILTER, this.glContext.LINEAR);
	this.glContext.texImage2D(this.glContext.TEXTURE_2D, 0, this.glContext.RGBA, this.fbo.width, this.fbo.height, 0, this.glContext.RGBA, this.glContext.UNSIGNED_BYTE, null);

	//
	this.glContext.framebufferTexture2D(this.glContext.FRAMEBUFFER, this.glContext.COLOR_ATTACHMENT0, this.glContext.TEXTURE_2D, this.renderTexture, 0);
	
	//unbind everything
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, null);
	this.unbind();
    }

    bind(){
	this.glContext.bindFramebuffer(this.glContext.FRAMEBUFFER, this.fbo);
    }

    unbind(){
	this.glContext.bindFramebuffer(this.glContext.FRAMEBUFFER, null);
    }
}
