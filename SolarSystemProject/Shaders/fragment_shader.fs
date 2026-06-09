#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_day;
uniform sampler2D texture_night;
uniform vec3 lightPos;
uniform int isEarth;
uniform int isSun;
uniform int isSkybox;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    vec4 dayCol = texture(texture_day, TexCoords);

    if(isSkybox == 1 || isSun == 1) {
        FragColor = dayCol;
    } 
    else if(isEarth == 1) {
        vec4 nightCol = texture(texture_night, TexCoords);
        float mixAmt = smoothstep(-0.1, 0.1, dot(norm, lightDir));
        FragColor = vec4(mix(nightCol.rgb, dayCol.rgb, mixAmt), 1.0);
    } 
    else {
        float diff = max(dot(norm, lightDir), 0.0);
        FragColor = vec4(dayCol.rgb * (diff + 0.15), 1.0);
    }
}