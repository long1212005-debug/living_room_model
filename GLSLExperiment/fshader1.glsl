#version 400

in vec4 color;
in vec3 N;
in vec3 posEye;

out vec4 fColor;

uniform vec4 objectColor;
uniform float brightness;

uniform vec4 mainLightPosition;
uniform vec4 lampLightPosition;

uniform int mainLightOn;
uniform int lampOn;

uniform vec4 tvLightPosition;
uniform int tvOn;

uniform float shininess;

void main()
{
    vec4 baseColor = color * objectColor;

    vec3 normal = normalize(N);
    vec3 viewDir = normalize(-posEye);

    // Anh sang nen co dinh theo brightness (phu thuoc gio trong ngay)
    vec3 ambient = vec3(0.40, 0.36, 0.30) * baseColor.rgb;
    vec3 result = ambient;

    // Den chinh trong phong
    if (mainLightOn == 1) {
        vec3 lightDir = normalize(mainLightPosition.xyz - posEye);
        float diff = max(dot(normal, lightDir), 0.0);

        vec3 diffuseColor = vec3(1.00, 0.86, 0.62);
        vec3 diffuse = diff * diffuseColor * baseColor.rgb;

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spec * vec3(0.70, 0.58, 0.38);

        result += diffuse * 0.95 + specular * 0.25;
    }

    // Den cay
    if (lampOn == 1) {
        vec3 lightDir2 = normalize(lampLightPosition.xyz - posEye);
        float diff2 = max(dot(normal, lightDir2), 0.0);

        vec3 lampColor = vec3(1.00, 0.68, 0.36);
        vec3 diffuse2 = diff2 * lampColor * baseColor.rgb;

        vec3 reflectDir2 = reflect(-lightDir2, normal);
        float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), shininess * 0.7);
        vec3 specular2 = spec2 * vec3(0.55, 0.38, 0.20);

        result += diffuse2 * 0.50 + specular2 * 0.18;
    }
if (tvOn == 1) {
        vec3 toLight3 = tvLightPosition.xyz - posEye;
        float dist3   = length(toLight3);
        vec3 lightDir3 = normalize(toLight3);

        // Suy giam theo khoang cach: vung gan man hinh sang ro,
        // cang xa cang mo nhanh (he so co the tinh chinh lai theo ty le scene)
        float atten3 = 1.0 / (1.0 + 0.6 * dist3 + 0.35 * dist3 * dist3);

        float diff3 = max(dot(normal, lightDir3), 0.0);
        vec3 tvColor = vec3(0.25, 0.45, 0.95); // anh sang xanh hat ra tu man hinh

        vec3 diffuse3  = diff3 * tvColor * baseColor.rgb * atten3;

        // mot chut specular de tao cam giac hat sang phan chieu len san/ban
        vec3 reflectDir3 = reflect(-lightDir3, normal);
        float spec3 = pow(max(dot(viewDir, reflectDir3), 0.0), shininess * 0.5);
        vec3 specular3 = spec3 * vec3(0.15, 0.25, 0.55) * atten3;

        result += diffuse3 * 1.5 + specular3 * 0.5;
    }

    result *= brightness;

    result.r *= 1.04;
    result.g *= 1.02;
    result.b *= 0.96;

    result = pow(result, vec3(0.95));
    result = clamp(result, 0.0, 1.0);

    fColor = vec4(result, baseColor.a);
}//