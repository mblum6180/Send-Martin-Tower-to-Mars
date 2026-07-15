#version 100

precision mediump float;

uniform sampler2D texture0;
uniform sampler2D u_previousFrame;
uniform vec2 u_sourceTexelSize;
uniform vec2 u_outputSize;
uniform float u_scanlineStrength;
uniform float u_scanlineCount;
uniform float u_scanlineSoftness;
uniform float u_bloomStrength;
uniform vec2 u_bloomRadius;
uniform float u_bloomThreshold;
uniform float u_phosphorWarmth;
uniform float u_curvatureAmount;
uniform float u_vignetteStrength;
uniform float u_intensity;
uniform float u_persistenceStrength;
uniform float u_rgbSeparation;
uniform float u_staticStrength;
uniform float u_time;

varying vec2 fragTexCoord;

const float PI = 3.14159265359;

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

// Cheap per-pixel hash for white-noise static; reseeded each frame via u_time.
float staticNoise(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 distortUv(vec2 uv) {
    vec2 centered = uv * 2.0 - 1.0;
    float dist = dot(centered, centered);
    centered += centered * dist * u_curvatureAmount;
    return centered * 0.5 + 0.5;
}

vec3 thresholdGlow(vec3 sampleColor) {
    float lum = luminance(sampleColor);
    float above = max(lum - u_bloomThreshold, 0.0);
    if (above <= 0.0) return vec3(0.0);
    float scale = above / max(1.0 - u_bloomThreshold, 0.0001);
    return sampleColor * scale;
}

vec3 bloomTap(vec2 uv) {
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return vec3(0.0);
    return thresholdGlow(texture2D(texture0, uv).rgb);
}

void main() {
    vec2 distortedUv = distortUv(fragTexCoord);
    if (distortedUv.x < 0.0 || distortedUv.x > 1.0 ||
        distortedUv.y < 0.0 || distortedUv.y > 1.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);   // transparent: let the bezel show in the curved border
        return;
    }

    vec4 baseSample = texture2D(texture0, distortedUv);
    vec3 baseColor = baseSample.rgb;
    vec2 bloomStep = max(u_bloomRadius, u_sourceTexelSize);

    vec3 bloom = vec3(0.0);
    float bloomHits = 0.0;

    vec3 tap;

    tap = bloomTap(distortedUv + vec2(0.0, -bloomStep.y));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(0.0, bloomStep.y));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(-bloomStep.x, 0.0));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(bloomStep.x, 0.0));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(-bloomStep.x, -bloomStep.y));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(bloomStep.x, -bloomStep.y));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(-bloomStep.x, bloomStep.y));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    tap = bloomTap(distortedUv + vec2(bloomStep.x, bloomStep.y));
    bloom += tap;
    bloomHits += step(0.0001, luminance(tap));

    if (bloomHits > 0.0)
        bloom /= bloomHits;

    bloom *= vec3(1.0 + 0.24 * u_phosphorWarmth,
                  1.0 + 0.12 * u_phosphorWarmth,
                  1.0 - 0.18 * u_phosphorWarmth);

    if (u_persistenceStrength > 0.0) {
        vec3 previousGlow = thresholdGlow(texture2D(u_previousFrame, distortedUv).rgb);
        bloom += previousGlow * u_persistenceStrength;
    }

    vec3 finalColor = baseColor + bloom * u_bloomStrength;

    if (u_scanlineStrength > 0.0) {
        float screenY = fragTexCoord.y * max(u_outputSize.y, 1.0);
        float scanCoord = (screenY / max(u_outputSize.y, 1.0)) * u_scanlineCount * PI;
        float scanMask = pow(abs(sin(scanCoord)), mix(8.0, 1.6, u_scanlineSoftness));
        float brightnessVisibility = mix(0.35, 1.0, clamp(luminance(finalColor), 0.0, 1.0));
        finalColor *= 1.0 - u_scanlineStrength * scanMask * brightnessVisibility;

        float scanRow = floor(fragTexCoord.y * u_scanlineCount);
        float grilleLift = 1.0 + 0.04 * (1.0 - step(0.5, mod(scanRow, 4.0)));
        finalColor *= grilleLift;
    }

    float dist = length((fragTexCoord - 0.5) * 2.0);
    finalColor *= clamp(1.0 - u_vignetteStrength * dist * dist, 0.0, 1.0);

    vec2 rgbOffset = vec2(u_rgbSeparation, 0.0);
    vec3 separated = vec3(
        texture2D(texture0, clamp(distortedUv + rgbOffset, 0.0, 1.0)).r,
        texture2D(texture0, distortedUv).g,
        texture2D(texture0, clamp(distortedUv - rgbOffset, 0.0, 1.0)).b
    );
    finalColor += (separated - baseColor) * (0.4 + 0.6 * u_intensity);

    // White-noise static — animated grain reseeded each frame.
    if (u_staticStrength > 0.0) {
        vec2 seed = fragTexCoord + vec2(fract(u_time * 1.137), fract(u_time * 0.731));
        float n = staticNoise(seed) - 0.5;
        finalColor += n * u_staticStrength;
    }

    gl_FragColor = vec4(clamp(finalColor, 0.0, 1.0), baseSample.a);
}
