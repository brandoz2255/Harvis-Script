import logging
import time
from datetime import datetime, timezone

from flask import Flask, jsonify, request

app = Flask(__name__)

START_TIME = time.time()

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
)
logger = logging.getLogger(__name__)


# Health check — returns service status
@app.route("/")
def index():
    logger.info("Health check requested")
    return jsonify({"status": "ok", "service": "miniapi"}), 200


# Uptime endpoint — returns how long the server has been running
@app.route("/health")
def health():
    uptime_seconds = round(time.time() - START_TIME, 2)
    logger.info("Uptime check requested")
    return jsonify({
        "status": "healthy",
        "uptime_seconds": uptime_seconds,
        "timestamp": datetime.now(timezone.utc).isoformat(),
    }), 200


# Stats endpoint — returns runtime statistics as JSON
@app.route("/stats")
def stats():
    uptime_seconds = round(time.time() - START_TIME, 2)
    logger.info("Stats requested")
    return jsonify({
        "uptime_seconds": uptime_seconds,
        "version": "1.0.0",
        "python_version": f"{__import__('sys').version}",
        "request_count": stats._count,
    }), 200


stats._count = 0


# Greeting endpoint — returns a personalized message based on the "name" query param
@app.route("/hello")
def hello():
    name = request.args.get("name")
    if not name:
        logger.warning("Missing 'name' query parameter on /hello")
        return jsonify({"error": "Missing required query parameter: name"}), 400

    logger.info("Greeting requested for '%s'", name)
    return jsonify({"message": f"Hello, {name}!"}), 200


# Catch-all for 404 errors
@app.errorhandler(404)
def not_found(error):
    logger.warning("404: %s not found", request.path)
    return jsonify({"error": "Not found"}), 404


# Catch-all for 500 errors
@app.errorhandler(500)
def internal_error(error):
    logger.error("500: Internal server error")
    return jsonify({"error": "Internal server error"}), 500


# Track request count per request
@app.before_request
def before_request():
    stats._count += 1


if __name__ == "__main__":
    logger.info("Starting miniapi on port 5000")
    app.run(host="0.0.0.0", port=5000, debug=False)
