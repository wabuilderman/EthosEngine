shaders=shader.frag shader.vert ui.frag ui.vert debug.frag debug.vert
exts=vert frag
shaderSPVs=$(foreach shader,$(shaders), $(shader).spv)

run: $(shaderSPVs)
	@echo Shaders Compiled.

.SECONDEXPANSION:
%.spv : src/$$*
	./glslc.exe src/$* -o $@

clean:
	@del /f "./*.spv"