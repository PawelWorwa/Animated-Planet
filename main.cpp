#include <SFML/Graphics.hpp>

const unsigned int WIDTH = 1024;
const unsigned int HEIGHT = 768;

const unsigned int randomSeed = static_cast<long unsigned int>(time( nullptr ));
std::default_random_engine randomDevice( randomSeed );
std::mt19937 seed( randomDevice());

unsigned int randomInt( unsigned int min, unsigned int max ) {
    return std::uniform_int_distribution< unsigned int >{min, max}( seed );
}

// ***************************
class Star {
    protected:
        enum Brightness {
                NONE   = 0,
                DIM    = 50,
                MEDIUM = 150,
                BRIGHT = 200
        };

        sf::CircleShape star;

    private:
        sf::Uint8 randomBrightness() {
            unsigned int randomValue = randomInt( 1, 3 );
            switch ( randomValue ) {
                case 1:
                    return Brightness::DIM;
                case 2:
                    return Brightness::MEDIUM;
                case 3:
                    return Brightness::BRIGHT;
                default:
                    return Brightness::NONE;
            }
        }

    public:
        Star() {
            star.setRadius( randomInt( 1, 2 ));
            sf::Color color = sf::Color::White;
            color.a = randomBrightness();
            star.setFillColor( color );
        }

        const sf::CircleShape &getStar() const {
            return star;
        }

        void setPosition( float x, float y ) {
            star.setPosition( x, y );
        }
};

// ***************************
class BlinkingStar : public Star {
    private:
        bool isBrightening = true;
        unsigned int currentBrightness = 0;
        unsigned int brightnessIncreasement;

    public:
        BlinkingStar() {
            brightnessIncreasement = randomInt( 1, 10 );
        }

        void blink() {
            if ( isBrightening ) {
                currentBrightness += brightnessIncreasement;
                if ( currentBrightness >= 255 ) {
                    currentBrightness = 255;
                    isBrightening = false;
                }

            } else {
                if ( currentBrightness >= brightnessIncreasement ) {
                    currentBrightness -= brightnessIncreasement;

                } else {
                    currentBrightness = 0;
                    isBrightening = true;
                }
            }

            sf::Color color = sf::Color::White;
            color.a = static_cast<sf::Uint8>(currentBrightness);
            star.setFillColor( color );
        }
};

// ***************************
class Background : public sf::Drawable {
    private:
        const unsigned int nr_of_stars = 300;

        sf::RenderTexture renderTexture;
        std::vector< Star > stars;

        void createStars() {
            for ( std::size_t i = 0; i < nr_of_stars; ++i ) {
                unsigned int xPos = randomInt( 0, WIDTH );
                unsigned int yPos = randomInt( 0, HEIGHT );

                Star star;
                star.setPosition( xPos, yPos );
                stars.push_back( star );
            }
        }

    public:
        Background() {
            renderTexture.create( WIDTH, HEIGHT );
        }

        void create() {
            createStars();
            renderTexture.clear( sf::Color::Black );
            for ( std::size_t i = 0; i < nr_of_stars; ++i ) {
                Star star = stars.at( i );
                renderTexture.draw( star.getStar());
            }

            renderTexture.display();
        }

        void draw( sf::RenderTarget &target, sf::RenderStates states ) const override {
            sf::Sprite sprite;
            sprite.setTexture( renderTexture.getTexture());
            target.draw( sprite, states );
        }
};

// ***************************
// https://gamedev.stackexchange.com/questions/147193/imitate-a-textured-sphere-in-2d
// http://clockworkchilli.com/blog/2_3d_shaders_in_a_2d_world
char shadowVertex[] =
        "uniform sampler2D texture;"
        "uniform float currentAngle;"

        "void main(void) {"
        "   const float PI = 3.141592653589793238462643383;"

        "   vec3 shadow = normalize( vec3( -0.4, -0.3, 0.4 ) );"
        "   vec2 textureCoordinates = gl_TexCoord[0].xy;"
        "   vec2 point = ( textureCoordinates - 0.5 )* 2.0;"
        "   float radius = sqrt( dot( point, point ));"

        "   vec3 normal = vec3( point.x,point.y, sqrt( 1.0 - point.x * point.x - point.y * point.y ));"
        //  ndotl - it's the dot product of a normalized light vector and a normalized surface normal
        "   float ndotl = max(0.0, dot(normal, shadow));"

        "   vec2 texCoords;"
        "   texCoords.x = ( 0.5 + atan( normal.x, normal.z ) / ( 2.0 * PI )) + currentAngle;"
        "   texCoords.y = asin( normal.y ) / PI - 0.5;"

        "   vec3 texColor = texture2D( texture, texCoords, 0.0 ).xyz;"
        "   vec3 lightColor = vec3( ndotl ) * texColor;"
        "   gl_FragColor = radius <= 1.0 ? vec4( lightColor, 1.0 ) : vec4( 0.0 );"
        "}";

// https://github.com/SFML/SFML/wiki/Source:-Radial-Gradient-Shader
const char radialGradientVertex[] =
        "uniform vec4 color;"
        "uniform float expand;"
        "uniform vec2 center;"
        "uniform float radius;"
        "uniform float windowHeight;"
        ""
        "void main(void) {"
        "   vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);"
        "   vec2 point = ( gl_FragCoord.xy - centerFromSfml ) / radius;"
        "   float radius = sqrt( dot( point, point ));"
        "   gl_FragColor = radius <= 1.0 ? mix( color, gl_Color, ( radius - expand ) / ( 1.0 - expand )) : vec4( 0.0 );"
        "}";

class Planet : public sf::Drawable {
    private:
        sf::CircleShape planet;
        sf::CircleShape atmosphere;
        sf::Texture planetTexture;

        sf::Shader planetShader;
        sf::Shader atmosphereShader;

        sf::Clock rotationClock;

        sf::Int32 rotationTick = 100;
        float planetSize       = 200.f;
        float rotationSpeed    = 0.01f;
        float currentAngle     = 0.0f;

        sf::Glsl::Vec4 getGlslColor( sf::Color color ) {
            float r = 1.f * color.r / 255.f;
            float g = 1.f * color.g / 255.f;
            float b = 1.f * color.b / 255.f;
            float a = 1.f * color.a / 255.f;

            return {r, g, b, a};
        }

        void prepareCircleShape( sf::CircleShape &shape, sf::Texture &texture ) {
            shape.setTexture( &texture );
            shape.setRadius( planetSize );
            shape.setOrigin( shape.getRadius(), shape.getRadius());
            shape.setPosition( WIDTH / 2.0f, HEIGHT / 2.0f );
        }

        void prepareSphereShader( sf::Shader &shader ) {
            shader.loadFromMemory( shadowVertex, sf::Shader::Fragment );
            shader.setUniform( "texture", sf::Shader::CurrentTexture );
        }

        void prepareAtmosphere( sf::Color atmosphereColor ) {
            float atmosphereRadius = planet.getRadius() + ( planet.getRadius() * 5.f / 100.f );
            atmosphere.setRadius( atmosphereRadius );
            atmosphere.setOrigin( atmosphere.getRadius(), atmosphere.getRadius());
            atmosphere.setPosition( planet.getPosition());
            atmosphere.setFillColor( sf::Color::Transparent );

            atmosphereShader.loadFromMemory( radialGradientVertex, sf::Shader::Fragment );
            atmosphereShader.setUniform( "color", getGlslColor( atmosphereColor ));
            atmosphereShader.setUniform( "center", atmosphere.getPosition());
            atmosphereShader.setUniform( "radius", atmosphere.getRadius());
            atmosphereShader.setUniform( "expand", 0.9f );
            atmosphereShader.setUniform( "windowHeight", static_cast<float>(HEIGHT));
        }

    public:
        Planet() {
            planetTexture.loadFromFile( "../world.png" );
            planetTexture.setRepeated( true );

            prepareCircleShape( planet, planetTexture );
            prepareSphereShader( planetShader );
            prepareAtmosphere( sf::Color( 135, 206, 235 ) );

            rotationClock.restart();
        }

        void rotate() {
            if ( rotationClock.getElapsedTime().asMilliseconds() > rotationTick ) {
                currentAngle += rotationSpeed;
                planetShader.setUniform( "currentAngle", currentAngle );
                rotationClock.restart();
            }
        }

        void draw( sf::RenderTarget &target, sf::RenderStates states ) const override {
            target.draw( atmosphere, &atmosphereShader );
            target.draw( planet, &planetShader );
        }
};

// ***************************
int main() {
    sf::RenderWindow window( sf::VideoMode( WIDTH, HEIGHT ), "Animated planet" );
    window.setFramerateLimit( 30u );

    Background starField;
    starField.create();

    std::vector< BlinkingStar > blinkingStars;
    int stars = 100;
    for ( std::size_t i = 0; i < stars; ++i ) {
        unsigned int xPos = randomInt( 0, WIDTH );
        unsigned int yPos = randomInt( 0, HEIGHT );

        BlinkingStar star;
        star.setPosition( xPos, yPos );
        blinkingStars.push_back( star );
    }

    Planet planet;

    while ( window.isOpen()) {
        sf::Event event;
        while ( window.pollEvent( event )) {
            if ( event.type == sf::Event::Closed ) {
                window.close();
            }
        }

        window.clear( sf::Color::Black );
        window.draw( starField );

        for ( std::size_t i = 0; i < stars; ++i ) {
            BlinkingStar &star = blinkingStars.at( i );
            star.blink();
            window.draw( star.getStar());
        }

        planet.rotate();
        window.draw( planet );

        window.display();
    }

    return 0;
}