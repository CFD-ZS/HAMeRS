#include "util/mixing_rules/equations_of_shear_viscosity/Chapman-Enskog/EquationOfShearViscosityChapmanEnskog.hpp"

#include <cmath>

/*
 * Print all characteristics of the equation of shear viscosity class.
 */
void
EquationOfShearViscosityChapmanEnskog::printClassData(
    std::ostream& os) const
{
    os << "\nPrint EquationOfShearViscosityChapmanEnskog object..."
       << std::endl;
    
    os << std::endl;
    
    os << "EquationOfShearViscosityChapmanEnskog: this = "
       << (EquationOfShearViscosityChapmanEnskog *)this
       << std::endl;
    
    os << "d_object_name = "
       << d_object_name
       << std::endl;
}


/*
 * Compute the shear viscosity.
 */
double
EquationOfShearViscosityChapmanEnskog::getShearViscosity(
    const double* const pressure,
    const double* const temperature,
    const std::vector<const double*>& molecular_properties) const
{
    NULL_USE(pressure);
    
#ifdef HAMERS_DEBUG_CHECK_DEV_ASSERTIONS
    TBOX_ASSERT(static_cast<int>(molecular_properties.size()) >= 3);
#endif
    
    double mu = double(0);
    
    const double& epsilon_by_k = *(molecular_properties[0]);
    const double& sigma = *(molecular_properties[1]);
    const double& M = *(molecular_properties[2]);
    
    const double& T = *temperature;
    
    const double A = double(1.16145);
    const double B = double(-0.14874);
    const double C = double(0.52487);
    const double D = double(-0.7732);
    const double E = double(2.16178);
    const double F = double(-2.43787);
    
    const double T_star = T/epsilon_by_k;
    const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
    
    mu = double(2.6693e-6)*sqrt(M*T)/(Omega*sigma*sigma);
    
    return mu;
}


/*
 * Compute the shear viscosity.
 */
void
EquationOfShearViscosityChapmanEnskog::computeShearViscosity(
    boost::shared_ptr<pdat::CellData<double> >& data_shear_viscosity,
    const boost::shared_ptr<pdat::CellData<double> >& data_pressure,
    const boost::shared_ptr<pdat::CellData<double> >& data_temperature,
    const std::vector<const double*>& molecular_properties,
    const hier::Box& domain) const
{
    NULL_USE(data_pressure);
    
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(data_shear_viscosity);
    TBOX_ASSERT(data_temperature);
    
    TBOX_ASSERT(static_cast<int>(molecular_properties.size()) >= 3);
#endif
    
    // Get the dimensions of box that covers the interior of patch.
    const hier::Box interior_box = data_shear_viscosity->getBox();
    const hier::IntVector interior_dims = interior_box.numberCells();
    
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(data_temperature->getBox().numberCells() == interior_dims);
#endif
    
    /*
     * Get the numbers of ghost cells and the dimensions of the ghost cell boxes.
     */
    
    const hier::IntVector num_ghosts_shear_viscosity = data_shear_viscosity->getGhostCellWidth();
    const hier::IntVector ghostcell_dims_shear_viscosity =
        data_shear_viscosity->getGhostBox().numberCells();
    
    const hier::IntVector num_ghosts_temperature = data_temperature->getGhostCellWidth();
    const hier::IntVector ghostcell_dims_temperature =
        data_temperature->getGhostBox().numberCells();
    
    /*
     * Get the local lower indices and number of cells in each direction of the domain.
     */
    
    hier::IntVector domain_lo(d_dim);
    hier::IntVector domain_dims(d_dim);
    
    if (domain.empty())
    {
        hier::IntVector num_ghosts_min(d_dim);
        
        num_ghosts_min = num_ghosts_shear_viscosity;
        num_ghosts_min = hier::IntVector::min(num_ghosts_temperature, num_ghosts_min);
        
        hier::Box ghost_box = interior_box;
        ghost_box.grow(num_ghosts_min);
        
        domain_lo = -num_ghosts_min;
        domain_dims = ghost_box.numberCells();
    }
    else
    {
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_shear_viscosity->getGhostBox().contains(domain));
        TBOX_ASSERT(data_temperature->getGhostBox().contains(domain));
#endif
        
        domain_lo = domain.lower() - interior_box.lower();
        domain_dims = domain.numberCells();
    }
    
    /*
     * Get the pointers to the cell data.
     */
    
    double* mu = data_shear_viscosity->getPointer(0);
    double* T = data_temperature->getPointer(0);
    
    const double& epsilon_by_k = *(molecular_properties[0]);
    const double& sigma = *(molecular_properties[1]);
    const double& M = *(molecular_properties[2]);
    
    const double A = double(1.16145);
    const double B = double(-0.14874);
    const double C = double(0.52487);
    const double D = double(-0.7732);
    const double E = double(2.16178);
    const double F = double(-2.43787);
    
    if (d_dim == tbox::Dimension(1))
    {
        /*
         * Get the local lower index, numbers of cells in each dimension and numbers of ghost cells.
         */
        
        const int domain_lo_0 = domain_lo[0];
        const int domain_dim_0 = domain_dims[0];
        
        const int num_ghosts_0_shear_viscosity = num_ghosts_shear_viscosity[0];
        const int num_ghosts_0_temperature = num_ghosts_temperature[0];
        
#ifdef HAMERS_ENABLE_SIMD
        #pragma omp simd
#endif
        for (int i = domain_lo_0; i < domain_lo_0 + domain_dim_0; i++)
        {
            // Compute the linear indices.
            const int idx_shear_viscosity = i + num_ghosts_0_shear_viscosity;
            const int idx_temperature = i + num_ghosts_0_temperature;
            
            const double T_star = T[idx_temperature]/epsilon_by_k;
            const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
            
            mu[idx_shear_viscosity] = double(2.6693e-6)*sqrt(M*T[idx_temperature])/(Omega*sigma*sigma);
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        /*
         * Get the local lower indices, numbers of cells in each dimension and numbers of ghost cells.
         */
        
        const int domain_lo_0 = domain_lo[0];
        const int domain_lo_1 = domain_lo[1];
        const int domain_dim_0 = domain_dims[0];
        const int domain_dim_1 = domain_dims[1];
        
        const int num_ghosts_0_shear_viscosity = num_ghosts_shear_viscosity[0];
        const int num_ghosts_1_shear_viscosity = num_ghosts_shear_viscosity[1];
        const int ghostcell_dim_0_shear_viscosity = ghostcell_dims_shear_viscosity[0];
        
        const int num_ghosts_0_temperature = num_ghosts_temperature[0];
        const int num_ghosts_1_temperature = num_ghosts_temperature[1];
        const int ghostcell_dim_0_temperature = ghostcell_dims_temperature[0];
        
        for (int j = domain_lo_1; j < domain_lo_1 + domain_dim_1; j++)
        {
#ifdef HAMERS_ENABLE_SIMD
            #pragma omp simd
#endif
            for (int i = domain_lo_0; i < domain_lo_0 + domain_dim_0; i++)
            {
                // Compute the linear indices.
                const int idx_shear_viscosity = (i + num_ghosts_0_shear_viscosity) +
                    (j + num_ghosts_1_shear_viscosity)*ghostcell_dim_0_shear_viscosity;
                
                const int idx_temperature = (i + num_ghosts_0_temperature) +
                    (j + num_ghosts_1_temperature)*ghostcell_dim_0_temperature;
                
                const double T_star = T[idx_temperature]/epsilon_by_k;
                const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
                
                mu[idx_shear_viscosity] = double(2.6693e-6)*sqrt(M*T[idx_temperature])/(Omega*sigma*sigma);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        /*
         * Get the local lower indices, numbers of cells in each dimension and numbers of ghost cells.
         */
        
        const int domain_lo_0 = domain_lo[0];
        const int domain_lo_1 = domain_lo[1];
        const int domain_lo_2 = domain_lo[2];
        const int domain_dim_0 = domain_dims[0];
        const int domain_dim_1 = domain_dims[1];
        const int domain_dim_2 = domain_dims[2];
        
        const int num_ghosts_0_shear_viscosity = num_ghosts_shear_viscosity[0];
        const int num_ghosts_1_shear_viscosity = num_ghosts_shear_viscosity[1];
        const int num_ghosts_2_shear_viscosity = num_ghosts_shear_viscosity[2];
        const int ghostcell_dim_0_shear_viscosity = ghostcell_dims_shear_viscosity[0];
        const int ghostcell_dim_1_shear_viscosity = ghostcell_dims_shear_viscosity[1];
        
        const int num_ghosts_0_temperature = num_ghosts_temperature[0];
        const int num_ghosts_1_temperature = num_ghosts_temperature[1];
        const int num_ghosts_2_temperature = num_ghosts_temperature[2];
        const int ghostcell_dim_0_temperature = ghostcell_dims_temperature[0];
        const int ghostcell_dim_1_temperature = ghostcell_dims_temperature[1];
        
        for (int k = domain_lo_2; k < domain_lo_2 + domain_dim_2; k++)
        {
            for (int j = domain_lo_1; j < domain_lo_1 + domain_dim_1; j++)
            {
#ifdef HAMERS_ENABLE_SIMD
                #pragma omp simd
#endif
                for (int i = domain_lo_0; i < domain_lo_0 + domain_dim_0; i++)
                {
                    // Compute the linear indices.
                    const int idx_shear_viscosity = (i + num_ghosts_0_shear_viscosity) +
                        (j + num_ghosts_1_shear_viscosity)*ghostcell_dim_0_shear_viscosity +
                        (k + num_ghosts_2_shear_viscosity)*ghostcell_dim_0_shear_viscosity*
                            ghostcell_dim_1_shear_viscosity;
                    
                    const int idx_temperature = (i + num_ghosts_0_temperature) +
                        (j + num_ghosts_1_temperature)*ghostcell_dim_0_temperature +
                        (k + num_ghosts_2_temperature)*ghostcell_dim_0_temperature*
                            ghostcell_dim_1_temperature;
                    
                    const double T_star = T[idx_temperature]/epsilon_by_k;
                    const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
                    
                    mu[idx_shear_viscosity] = double(2.6693e-6)*sqrt(M*T[idx_temperature])/(Omega*sigma*sigma);
                }
            }
        }
    }
}


/*
 * Compute the shear viscosity.
 */
void
EquationOfShearViscosityChapmanEnskog::computeShearViscosity(
    boost::shared_ptr<pdat::CellData<double> >& data_shear_viscosity,
    const boost::shared_ptr<pdat::CellData<double> >& data_pressure,
    const boost::shared_ptr<pdat::CellData<double> >& data_temperature,
    const boost::shared_ptr<pdat::CellData<double> >& data_molecular_properties,
    const hier::Box& domain) const
{
    NULL_USE(data_pressure);
    
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(data_shear_viscosity);
    TBOX_ASSERT(data_temperature);
    TBOX_ASSERT(data_molecular_properties);
    
    TBOX_ASSERT(data_molecular_properties->getDepth() >= 3);
#endif
    
    // Get the dimensions of box that covers the interior of patch.
    const hier::Box interior_box = data_shear_viscosity->getBox();
    const hier::IntVector interior_dims = interior_box.numberCells();
    
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(data_temperature->getBox().numberCells() == interior_dims);
    TBOX_ASSERT(data_molecular_properties->getBox().numberCells() == interior_dims);
#endif
    
    /*
     * Get the numbers of ghost cells and the dimensions of the ghost cell boxes.
     */
    
    const hier::IntVector num_ghosts_shear_viscosity = data_shear_viscosity->getGhostCellWidth();
    const hier::IntVector ghostcell_dims_shear_viscosity =
        data_shear_viscosity->getGhostBox().numberCells();
    
    const hier::IntVector num_ghosts_temperature = data_temperature->getGhostCellWidth();
    const hier::IntVector ghostcell_dims_temperature =
        data_temperature->getGhostBox().numberCells();
    
    const hier::IntVector num_ghosts_molecular_properties = data_molecular_properties->getGhostCellWidth();
    const hier::IntVector ghostcell_dims_molecular_properties =
        data_molecular_properties->getGhostBox().numberCells();
    
    /*
     * Get the local lower indices and number of cells in each direction of the domain.
     */
    
    hier::IntVector domain_lo(d_dim);
    hier::IntVector domain_dims(d_dim);
    
    if (domain.empty())
    {
        hier::IntVector num_ghosts_min(d_dim);
        
        num_ghosts_min = num_ghosts_shear_viscosity;
        num_ghosts_min = hier::IntVector::min(num_ghosts_temperature, num_ghosts_min);
        num_ghosts_min = hier::IntVector::min(num_ghosts_molecular_properties, num_ghosts_min);
        
        hier::Box ghost_box = interior_box;
        ghost_box.grow(num_ghosts_min);
        
        domain_lo = -num_ghosts_min;
        domain_dims = ghost_box.numberCells();
    }
    else
    {
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_shear_viscosity->getGhostBox().contains(domain));
        TBOX_ASSERT(data_temperature->getGhostBox().contains(domain));
        TBOX_ASSERT(data_molecular_properties->getGhostBox().contains(domain));
#endif
        
        domain_lo = domain.lower() - interior_box.lower();
        domain_dims = domain.numberCells();
    }
    
    /*
     * Get the pointers to the cell data.
     */
    
    double* mu = data_shear_viscosity->getPointer(0);
    double* T = data_temperature->getPointer(0);
    
    double* epsilon_by_k = data_molecular_properties->getPointer(0);
    double* sigma = data_molecular_properties->getPointer(1);
    double* M = data_molecular_properties->getPointer(2);
    
    const double A = double(1.16145);
    const double B = double(-0.14874);
    const double C = double(0.52487);
    const double D = double(-0.7732);
    const double E = double(2.16178);
    const double F = double(-2.43787);
    
    if (d_dim == tbox::Dimension(1))
    {
        /*
         * Get the local lower index, numbers of cells in each dimension and numbers of ghost cells.
         */
        
        const int domain_lo_0 = domain_lo[0];
        const int domain_dim_0 = domain_dims[0];
        
        const int num_ghosts_0_shear_viscosity = num_ghosts_shear_viscosity[0];
        const int num_ghosts_0_temperature = num_ghosts_temperature[0];
        const int num_ghosts_0_molecular_properties = num_ghosts_molecular_properties[0];
        
#ifdef HAMERS_ENABLE_SIMD
        #pragma omp simd
#endif
        for (int i = domain_lo_0; i < domain_lo_0 + domain_dim_0; i++)
        {
            // Compute the linear indices.
            const int idx_shear_viscosity = i + num_ghosts_0_shear_viscosity;
            const int idx_temperature = i + num_ghosts_0_temperature;
            const int idx_molecular_properties = i + num_ghosts_0_molecular_properties;
            
            const double T_star = T[idx_temperature]/epsilon_by_k[idx_molecular_properties];
            const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
            
            mu[idx_shear_viscosity] = double(2.6693e-6)*sqrt(M[idx_molecular_properties]*T[idx_temperature])/
                (Omega*sigma[idx_molecular_properties]*sigma[idx_molecular_properties]);
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        /*
         * Get the local lower indices, numbers of cells in each dimension and numbers of ghost cells.
         */
        
        const int domain_lo_0 = domain_lo[0];
        const int domain_lo_1 = domain_lo[1];
        const int domain_dim_0 = domain_dims[0];
        const int domain_dim_1 = domain_dims[1];
        
        const int num_ghosts_0_shear_viscosity = num_ghosts_shear_viscosity[0];
        const int num_ghosts_1_shear_viscosity = num_ghosts_shear_viscosity[1];
        const int ghostcell_dim_0_shear_viscosity = ghostcell_dims_shear_viscosity[0];
        
        const int num_ghosts_0_temperature = num_ghosts_temperature[0];
        const int num_ghosts_1_temperature = num_ghosts_temperature[1];
        const int ghostcell_dim_0_temperature = ghostcell_dims_temperature[0];
        
        const int num_ghosts_0_molecular_properties = num_ghosts_molecular_properties[0];
        const int num_ghosts_1_molecular_properties = num_ghosts_molecular_properties[1];
        const int ghostcell_dim_0_molecular_properties = ghostcell_dims_molecular_properties[0];
        
        for (int j = domain_lo_1; j < domain_lo_1 + domain_dim_1; j++)
        {
#ifdef HAMERS_ENABLE_SIMD
            #pragma omp simd
#endif
            for (int i = domain_lo_0; i < domain_lo_0 + domain_dim_0; i++)
            {
                // Compute the linear indices.
                const int idx_shear_viscosity = (i + num_ghosts_0_shear_viscosity) +
                    (j + num_ghosts_1_shear_viscosity)*ghostcell_dim_0_shear_viscosity;
                
                const int idx_temperature = (i + num_ghosts_0_temperature) +
                    (j + num_ghosts_1_temperature)*ghostcell_dim_0_temperature;
                
                const int idx_molecular_properties = (i + num_ghosts_0_molecular_properties) +
                    (j + num_ghosts_1_molecular_properties)*ghostcell_dim_0_molecular_properties;
                
                const double T_star = T[idx_temperature]/epsilon_by_k[idx_molecular_properties];
                const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
                
                mu[idx_shear_viscosity] = double(2.6693e-6)*sqrt(M[idx_molecular_properties]*T[idx_temperature])/
                    (Omega*sigma[idx_molecular_properties]*sigma[idx_molecular_properties]);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        /*
         * Get the local lower indices, numbers of cells in each dimension and numbers of ghost cells.
         */
        
        const int domain_lo_0 = domain_lo[0];
        const int domain_lo_1 = domain_lo[1];
        const int domain_lo_2 = domain_lo[2];
        const int domain_dim_0 = domain_dims[0];
        const int domain_dim_1 = domain_dims[1];
        const int domain_dim_2 = domain_dims[2];
        
        const int num_ghosts_0_shear_viscosity = num_ghosts_shear_viscosity[0];
        const int num_ghosts_1_shear_viscosity = num_ghosts_shear_viscosity[1];
        const int num_ghosts_2_shear_viscosity = num_ghosts_shear_viscosity[2];
        const int ghostcell_dim_0_shear_viscosity = ghostcell_dims_shear_viscosity[0];
        const int ghostcell_dim_1_shear_viscosity = ghostcell_dims_shear_viscosity[1];
        
        const int num_ghosts_0_temperature = num_ghosts_temperature[0];
        const int num_ghosts_1_temperature = num_ghosts_temperature[1];
        const int num_ghosts_2_temperature = num_ghosts_temperature[2];
        const int ghostcell_dim_0_temperature = ghostcell_dims_temperature[0];
        const int ghostcell_dim_1_temperature = ghostcell_dims_temperature[1];
        
        const int num_ghosts_0_molecular_properties = num_ghosts_molecular_properties[0];
        const int num_ghosts_1_molecular_properties = num_ghosts_molecular_properties[1];
        const int num_ghosts_2_molecular_properties = num_ghosts_molecular_properties[2];
        const int ghostcell_dim_0_molecular_properties = ghostcell_dims_molecular_properties[0];
        const int ghostcell_dim_1_molecular_properties = ghostcell_dims_molecular_properties[1];
        
        for (int k = domain_lo_2; k < domain_lo_2 + domain_dim_2; k++)
        {
            for (int j = domain_lo_1; j < domain_lo_1 + domain_dim_1; j++)
            {
#ifdef HAMERS_ENABLE_SIMD
                #pragma omp simd
#endif
                for (int i = domain_lo_0; i < domain_lo_0 + domain_dim_0; i++)
                {
                    // Compute the linear indices.
                    const int idx_shear_viscosity = (i + num_ghosts_0_shear_viscosity) +
                        (j + num_ghosts_1_shear_viscosity)*ghostcell_dim_0_shear_viscosity +
                        (k + num_ghosts_2_shear_viscosity)*ghostcell_dim_0_shear_viscosity*
                            ghostcell_dim_1_shear_viscosity;
                    
                    const int idx_temperature = (i + num_ghosts_0_temperature) +
                        (j + num_ghosts_1_temperature)*ghostcell_dim_0_temperature +
                        (k + num_ghosts_2_temperature)*ghostcell_dim_0_temperature*
                            ghostcell_dim_1_temperature;
                    
                    const int idx_molecular_properties = (i + num_ghosts_0_molecular_properties) +
                        (j + num_ghosts_1_molecular_properties)*ghostcell_dim_0_molecular_properties +
                        (k + num_ghosts_2_molecular_properties)*ghostcell_dim_0_molecular_properties*
                            ghostcell_dim_1_molecular_properties;
                    
                    const double T_star = T[idx_temperature]/epsilon_by_k[idx_molecular_properties];
                    const double Omega = A*pow(T_star, B) + C*exp(D*T_star) + E*exp(F*T_star);
                    
                    mu[idx_shear_viscosity] = double(2.6693e-6)*sqrt(M[idx_molecular_properties]*T[idx_temperature])/
                        (Omega*sigma[idx_molecular_properties]*sigma[idx_molecular_properties]);
                }
            }
        }
    }
}
