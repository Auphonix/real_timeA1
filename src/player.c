#include "player.h"
#include "gl.h"

#define ROT_AMOUNT (M_PI / 4.0) // amount that rotation will change each frame the controls are pressed
#define SPEED_AMOUNT 1.0 // amount that speed will change each frame the controls are pressed

/*
 * Update the player's position and velocity with frametime dt
 */
static void
integratePlayer(Player* player, float dt)
{
  // update velocity with gravity
  player->vel.y -= player->g * dt;

  // this gives a pretty reasonable integration of acceleration with each step
  player->pos.y += player->vel.y * dt + 0.5f * player->g * dt * dt;
  player->pos.x += player->vel.x * dt;
  player->pos.z += player->vel.z * dt;

  // when we have finished jumping, reset for next jump
  if (player->pos.y <= 0) {
    player->jump = false;
    player->pos.y = 0;
    player->initPos = player->pos;
  }
}

/*
 * Initialise the player
 */
void
initPlayer(Player* player, DrawingFlags* flags)
{
  player->pos = (Vec3f) { 0, 0, 4 };
  player->initPos = player->pos;

  player->xRot = M_PI / 4.0;
  player->yRot = M_PI;
  player->speed = 2.0;
  player->size = 0.1;
  player->g = 9.8;
  player->jump = false;

  generatePlayerGeometry(player, flags->tess);
  player->material = (Material) { { 0.2, 0.2, 0.2, 0 }, { 0.1, 0.5, 0.9, 0 }, { 1, 1, 1, 0 }, 50 };
}

/*
 * Cleanup any memory used by the player
 */
void
destroyPlayer(Player* player)
{
  destroyMesh(player->mesh);
}

/*
 * Generate the player mesh, or regenerate it if it already exists.
 * This should be called whenever the tesselation is increased or decreased
 */
void
generatePlayerGeometry(Player* player, size_t segments[])
{
  if (player->mesh)
    destroyMesh(player->mesh);
  player->mesh = createSphere(segments[0], segments[1]);
}

/*
 * Update the player's state for frametime dt.
 * If we aren't jumping, update speed and rotation from controls,
 * otherwise animate the next step of our jump
 */
void
updatePlayer(Player* player, float dt, Controls* controls)
{
  if (!player->jump) {
    // process controls
    if (controls->up)
      player->speed += SPEED_AMOUNT * dt;
    if (controls->down)
      player->speed -= SPEED_AMOUNT * dt;
    if (controls->left)
      player->xRot += ROT_AMOUNT * dt;
    if (controls->right)
      player->xRot -= ROT_AMOUNT * dt;
    if (controls->turnLeft)
      player->yRot += ROT_AMOUNT * dt;
    if (controls->turnRight)
      player->yRot -= ROT_AMOUNT * dt;
    if (controls->jump)
      player->jump = true;

    // dont let the player jump backwards or into the floor
    player->speed = max(0, player->speed);
    player->xRot = clamp(player->xRot, 0, M_PI);

    // set our initial velocity from speed and rotation
    player->initVel.x = cosf(player->xRot) * sinf(player->yRot);
    player->initVel.y = sinf(player->xRot);
    player->initVel.z = cosf(player->yRot) * cosf(player->xRot);

    player->initVel = mulVec3f(player->initVel, player->speed);
    player->vel = player->initVel;
  } else {
    integratePlayer(player, dt);
  }
}

/*
 * Draw the player's mesh, as well as a parabola showing our jump arc
 * and a visualization of our current velocity
 */
void
renderPlayer(Player* player, DrawingFlags* flags)
{
  glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);

  // draw the parabola from the starting point of the jump
  glPushMatrix();
  glTranslatef(player->initPos.x, player->initPos.y, player->initPos.z);
  drawParabola(BLUE, player->initVel, player->g, flags);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(player->pos.x, player->pos.y, player->pos.z);

  // draw the player mesh at our current position
  glPushMatrix();
  glRotatef(RADDEG(player->yRot), 0, 1, 0);
  glScalef(player->size, player->size, player->size);
  applyMaterial(&player->material);
  glColor3f(0.1, 0.5, 0.9);
  renderMesh(player->mesh, flags);
  glPopMatrix();

  // draw the visualization of the player's velocity at our current position
  drawLine(PURPLE, (Vec3f) { 0, 0, 0 }, mulVec3f(player->vel, 0.1));
  glPopMatrix();

  glPopAttrib();
}
