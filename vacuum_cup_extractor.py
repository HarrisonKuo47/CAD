"""
Vacuum cup extractor module (formerly coordinate_matcher.py).
This module handles extraction and processing of vacuum cup coordinates.
"""

# Import the improved geometric detector from the new location
from geo_vacuum_detector import GeometricVacuumCupDetector
from vacuum_cup_detector import ContourVacuumCupDetector, AdaptiveVacuumCupDetector, HybridVacuumCupDetector

class VacuumCupExtractor:
    """Main class for extracting vacuum cup coordinates."""
    
    def __init__(self):
        # Using the improved GeometricVacuumCupDetector from geo_vacuum_detector
        self.geometric_detector = GeometricVacuumCupDetector()
        self.contour_detector = ContourVacuumCupDetector()
        self.adaptive_detector = AdaptiveVacuumCupDetector()
        self.hybrid_detector = HybridVacuumCupDetector()
    
    def extract_coordinates(self, image):
        """Extract vacuum cup coordinates from the given image."""
        # Implementation would use the detectors
        pass